#include "./sylar/sylar.h"

static hr::Logger::ptr g_logger = HR_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    HR_LOG_INFO(g_logger) << "test in fiber s_count" << s_count;

    sleep(1);

    if(--s_count >= 0) {
        //子协程创建任务，指定线程执行
        hr::Scheduler::GetThis()->schedule(&test_fiber, hr::GetThreadId());
    }
}

int main(int argc, char** argv) {
    HR_LOG_INFO(g_logger) << "main";
    hr::Scheduler sc(3, false, "test");
    //创建线程池开始调度
    sc.start();
    sleep(2);
    HR_LOG_INFO(g_logger) << "schedule";
    //主协程创建任务
    sc.schedule(&test_fiber);
    sc.stop();
    HR_LOG_INFO(g_logger) << "over";
    return 0;
}