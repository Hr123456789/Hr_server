#include "sylar/sylar.h"

hr::Logger::ptr g_logger = HR_LOG_ROOT();

void run_in_fiber() {
    HR_LOG_INFO(g_logger) << "run_in_fiber begin";
    hr::Fiber::YieldToHold();
    HR_LOG_INFO(g_logger) << "run_in_fiber end";
    hr::Fiber::YieldToHold();
}

void test_fiber() {
    HR_LOG_INFO(g_logger) << "main begin -1";
    {
        hr::Fiber::GetThis();
        HR_LOG_INFO(g_logger) << "main begin";
        hr::Fiber::ptr fiber(new hr::Fiber(run_in_fiber));
        fiber->call();
        HR_LOG_INFO(g_logger) << "main after swapIn";
        fiber->call();
        HR_LOG_INFO(g_logger) << "main after end";
        fiber->call();
    }
    HR_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    hr::Thread::SetName("main");

    std::vector<hr::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(hr::Thread::ptr(new hr::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}