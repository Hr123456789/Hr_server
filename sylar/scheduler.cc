#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace hr {

static hr::Logger::ptr g_logger = HR_LOG_NAME("system");

//调度器，同一调度器下的所有线程都指向同一个调度器实例
static thread_local Scheduler* t_scheduler = nullptr;
//当前线程的调度协程，每个线程独一份，包括caller线程
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name){
    SYLAR_ASSERT(threads > 0);

    //使用use_caller
    if(use_caller) {
        //初始化主协程，主协程等于当前caller协程
        hr::Fiber::GetThis();
        --threads;

        SYLAR_ASSERT(GetThis() == nullptr);

        t_scheduler = this;

        //为caller线程初始化一个调度协程
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        hr::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = hr::GetThreadId();
        m_threadIds.push_back(m_rootThread);

    } else {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    SYLAR_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

//返回当前线程的调度协程
Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

//创建线程池
void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return;
    }
    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                            ,m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}

void Scheduler::stop() {
    m_autoStop = true;
    if(m_rootFiber
            && m_threadCount == 0
            && (m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        HR_LOG_INFO(g_logger) << this << "stopped";
        m_stopping = true;

        if(stopping()) {
            return;
        }
    }

    if(m_rootThread != -1) {
        SYLAR_ASSERT(GetThis() == this);
    } else {
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        if(!stopping()) {
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
    
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    HR_LOG_DEBUG(g_logger) << m_name << "run";
    //是否使用hook
    set_hook_enable(true);
    //设置当前线程的调度器，即将线程的t_scheduler指向当前的调度器
    setThis();
    //如果不是use_caller线程
    if(hr::GetThreadId() != m_rootThread) {
        //将调度协程上下文保存在t_scheduler_fiber里
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    //创建空闲协程并没有执行
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true) {
        //重置数据
        ft.reset();
        //是否需要通知其他线程进行任务调度
        bool tickle_me = false;
        //是否活跃
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                //指定其他线程执行
                if(it->thread != -1 && it->thread != hr::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                //如果在执行中就跳过
                SYLAR_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                //取出一个任务
                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            // 当前线程拿完一个任务后，发现任务队列还有剩余，那么tickle一下其他线程
            tickle_me |= it != m_fibers.end();
        }

        //如果需要通知就通知其他线程
        if(tickle_me) {
            tickle();
        }

        //如果是协程且协程的状态不等于TERM和EXCEPT就执行
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            //执行任务队列的协程
            ft.fiber->swapIn();
            //如果返回，要么是执行完了要么是暂停了，所以工作线程数量减一
            --m_activeThreadCount;
            //如果状态是READY再添加到任务队列
            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            //如果执行完就清空数据
            ft.reset();
        } else if(ft.cb) {
            //把回调函数封装成协程，因为只有携程可以中途执行
            //若协程不为空重置为回调函数的协程，如果为空new一个新协程
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            //将ft数据清空
            ft.reset();
            //执行回调函数的协程，执行完后将协程重置，状态为TERM
            cb_fiber->swapIn();
            //同上
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {//if(cb_fiber->getState() != Fiber::TERM) {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {
            //如果是已经在执行的
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            //如果空闲协程状态为TERM说明调度结束退出循环
            if(idle_fiber->getState() == Fiber::TERM) {
                HR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            //空闲线程数加一
            ++m_idleThreadCount;
            //执行空闲协程，被唤醒后空闲协程重置，状态为TERM
            idle_fiber->swapIn();
            //出来说明被唤醒有新的任务了，空闲线程数减一
            --m_idleThreadCount;
            //将空闲线程的状态设置为HOLD
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }

    }
}

void Scheduler::tickle() {
    HR_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    HR_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        hr::Fiber::YieldToHold();
    }
}

void Scheduler::switchTo(int thread) {
    SYLAR_ASSERT(Scheduler::GetThis() != nullptr);
    if(Scheduler::GetThis() == this) {
        if(thread == -1 || thread == hr::GetThreadId()) {
            return;
        }
    }
    schedule(Fiber::GetThis(), thread);
    Fiber::YieldToHold();
}

//输出信息
std::ostream& Scheduler::dump(std::ostream& os) {
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleThreadCount
       << " stopping=" << m_stopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
    m_caller = Scheduler::GetThis();
    if(target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher() {
    if(m_caller) {
        m_caller->switchTo();
    }
}

}