#include "hook.h"
#include <dlfcn.h>

#include "config.h"
#include "log.h"
#include "fiber.h"
#include "iomanager.h"
#include "fd_manager.h"
#include "macro.h"


hr::Logger::ptr g_logger = HR_LOG_NAME("system");
namespace hr {

static hr::ConfigVar<int>::ptr g_tcp_connect_timeout =
    hr::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

//初始化将原函数和函数名绑定
void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
                HR_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                         << old_value << " to " << new_value;
                s_connect_timeout = new_value;
        });
    }
};

//静态变量初始化是在main函数之前
//所以想让函数在main函数之前初始化可以设置为静态变量
static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}
}

struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {
    if(!hr::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    //std::forward 实现完美转发，作用就是保持传参参数属性不变，
    //如果原来的值是左值，经std::forward处理后该值还是左值；
    //如果原来的值是右值，经std::forward处理后它还是右值。
    hr::FdCtx::ptr ctx = hr::FdMgr::GetInstance()->get(fd);
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    while(n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }

    if(n == -1 && errno == EAGAIN) {
        hr::IOManager* iom = hr::IOManager::GetThis();
        hr::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(to != (uint64_t)-1) {
            //添加条件定时器
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                //lock函数是取出shared_ptr指向的对象
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (hr::IOManager::Event)(event));
            }, winfo);
        }

        int rt = iom->addEvent(fd, (hr::IOManager::Event)(event));
        if(SYLAR_UNLIKELY(rt)) {
            HR_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else{
            hr::Fiber::YieldToHold();
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}

extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if(!hr::t_hook_enable) {
        return sleep_f(seconds);
    }

    //初始化协程
    hr::Fiber::ptr fiber = hr::Fiber::GetThis();
    hr::IOManager* iom = hr::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(hr::Scheduler::*)
            (hr::Fiber::ptr, int thread))&hr::IOManager::schedule
            ,iom, fiber, -1));
    hr::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if(!hr::t_hook_enable) {
        return usleep_f(usec);
    }
    hr::Fiber::ptr fiber = hr::Fiber::GetThis();
    hr::IOManager* iom = hr::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(hr::Scheduler::*)
            (hr::Fiber::ptr, int thread))&hr::IOManager::schedule
            ,iom, fiber, -1));
    hr::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!hr::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    hr::Fiber::ptr fiber = hr::Fiber::GetThis();
    hr::IOManager* iom = hr::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(hr::Scheduler::*)
            (hr::Fiber::ptr, int thread))&hr::IOManager::schedule
            ,iom, fiber, -1));
    hr::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if(!hr::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    hr::FdMgr::GetInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
    if(!hr::t_hook_enable) {
        return connect_f(fd, addr, addrlen);
    }
    hr::FdCtx::ptr ctx = hr::FdMgr::GetInstance()->get(fd);
    //文件关闭
    if(!ctx || ctx->isClose()) {
        /*
        文件编号错误。 有两个可能的原因：
        1) 指定的文件描述符不是有效值或未引用打开的文件。 
        2) 尝试写入到已打开进行只读访问的文件或设备。
        */
        errno = EBADF;
        return -1;
    }

    //不是socket套接字
    if(!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }

    //用户自己设置了非阻塞
    if(ctx->getUserNonblock()) {
        return connect_f(fd, addr, addrlen);
    }

    int n = connect_f(fd, addr, addrlen);
    if(n == 0) {
        return 0;
    } 
    //	操作正在进行。
    else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    hr::IOManager* iom = hr::IOManager::GetThis();
    hr::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, hr::IOManager::WRITE);
        }, winfo);
    }

    int rt = iom->addEvent(fd, hr::IOManager::WRITE);
    if(rt == 0) {
        hr::Fiber::YieldToHold();
        if(timer) {
            timer->cancel();
        }
        if(tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        HR_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    /*
    int getsockopt(int socket, int level, int option_name,
            void *restrict option_value, socklen_t *restrict option_len);
 功能：获取一个套接字的选项
 参数：
     socket：文件描述符
     level：协议层次
            SOL_SOCKET 套接字层次
            IPPROTO_IP ip层次
            IPPROTO_TCP TCP层次
    option_name：选项的名称（套接字层次）
            SO_BROADCAST 是否允许发送广播信息
            SO_REUSEADDR 是否允许重复使用本地地址
           SO_SNDBUF 获取发送缓冲区长度
           SO_RCVBUF 获取接收缓冲区长度    

           SO_RCVTIMEO 获取接收超时时间
           SO_SNDTIMEO 获取发送超时时间
    option_value：获取到的选项的值
    option_len：value的长度
 返回值：
    成功：0
    失败：-1
    */
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, hr::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(s, accept_f, "accept", hr::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        hr::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", hr::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", hr::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", hr::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", hr::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", hr::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", hr::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", hr::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", hr::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", hr::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", hr::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if(!hr::t_hook_enable) {
        return close_f(fd);
    }

    hr::FdCtx::ptr ctx = hr::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        //取消fd上监听的所有事件会触发一次回调
        auto iom = hr::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        hr::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ...) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                hr::FdCtx::ptr ctx = hr::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonbblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                hr::FdCtx::ptr ctx = hr::FdMgr::GetInstance()->get(fd);
                if(!ctx ||ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        hr::FdCtx::ptr ctx = hr::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!hr::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            hr::FdCtx::ptr ctx = hr::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}

