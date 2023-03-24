#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
#include "scheduler.h"
#include <atomic>

namespace hr {

//日志器
static Logger::ptr g_logger = HR_LOG_NAME("system");

//原子操作
//协程id
static std::atomic<uint64_t> s_fiber_id {0};
//协程总数
static std::atomic<uint64_t> s_fiber_count {0};

//线程局部变量
//当前运行的协程
static thread_local Fiber* t_fiber = nullptr;
//主协程
static thread_local Fiber::ptr t_threadFiber = nullptr;

//栈大小写入配置文件
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

//栈内存分配器
class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

//取别名
using StackAllocator = MallocStackAllocator;

//获取协程id
uint64_t Fiber::GetFiberId() {
    if(t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

//默认构造函数
Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        //报错
        SYLAR_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;

    //HR_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

//有参构造函数
Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) 
    :m_id(++s_fiber_count)
    ,m_cb(cb){
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        //报错
        SYLAR_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    //不使用主线程的协程
    if(!use_caller) {
        //回到主协程
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        //将当前协程的上下文与执行函数绑定
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
    HR_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}

//析构函数
Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        SYLAR_ASSERT(m_state == TERM
                || m_state == EXCEPT
                || m_state == INIT);
        
        //释放内存
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    HR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id
                           << "total = " << s_fiber_count;
}

//重置协程函数，并重置状态
//INIT, TERM, EXCEPT<
void Fiber::reset(std::function<void()> cb) {
    SYLAR_ASSERT(m_stack);
    SYLAR_ASSERT(m_state == TERM
            || m_state == EXCEPT
            || m_state == INIT);
    m_cb = cb;
    if(getcontext(&m_ctx)) {
        //报错
        SYLAR_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

//从主协程切换到执行线程
void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        //报错
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//从当前协程返回到主线程
void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        //报错
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//当前协程开始执行
void Fiber::swapIn() {
    SetThis(this);
    //确保当前运行的不是当前协程
    SYLAR_ASSERT(m_state != EXEC);
    m_state = EXEC;
    //从调度协程切换到当前协程
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        //报错
        SYLAR_ASSERT2(false, "swapcontext");
    }

}

//从当前协程切换到调度协程
void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        //报错
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//设置当前协程
//保证t_fiber永远指向当前运行协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

//返回当前运行的协程
//shared_from_this()会是引用+1
Fiber::ptr Fiber::GetThis() {
    if(t_fiber) {
        return t_fiber->shared_from_this();
    }
    //当前线程没有创建协程
    Fiber::ptr main_fiber(new Fiber);
    //创建的是主协程
    SYLAR_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

//将当前协程切换到后台，并且设置为Ready状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    //当前协程是否正在执行中
    SYLAR_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

//将当前协程切换到后台，并且设置为HOLD状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    //当前协程是否在执行中
    SYLAR_ASSERT(cur->m_state == EXEC);
    //cur->back();
    //返回到调度器
    cur->swapOut();
}

//总协程数
uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

//协程执行函数
void Fiber::MainFunc() {
    //初始化主协程
    Fiber::ptr cur = GetThis();
    //当前协程是否为空
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        HR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << hr::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        HR_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << hr::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    //返回到调度协程
    //raw_ptr->swapOut();
    raw_ptr->swapOut();

    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

//
void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        HR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << hr::BacktraceToString();
    } catch (...) {
       cur->m_state = EXCEPT;
        HR_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << hr::BacktraceToString(); 
    }

    auto raw_ptr = cur.get();
    cur.reset();
    //返回调度协程
    raw_ptr->back();

    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

}
