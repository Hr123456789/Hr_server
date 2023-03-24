#include "../sylar/sylar.h"
#include <unistd.h>

hr::Logger::ptr g_logger = HR_LOG_ROOT();

int count = 0;
//sylar::RWMutex s_mutex;
hr::Mutex s_mutex;

void fun1() {
    HR_LOG_INFO(g_logger) << "name: " << hr::Thread::GetName()
                             << " this.name: " << hr::Thread::GetThis()->getName()
                             << " id: " << hr::GetThreadId()
                             << " this.id: " << hr::Thread::GetThis()->getId();

    for(int i = 0; i < 100000; ++i) {
        //sylar::RWMutex::WriteLock lock(s_mutex);
        hr::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2() {
    while(true) {
        HR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while(true) {
        HR_LOG_INFO(g_logger) << "========================================";
    }
}

int main(int argc, char** argv) {
    HR_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("../bin/config/log.yml");
    hr::Config::LoadFromYaml(root);

    std::vector<hr::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        hr::Thread::ptr thr(new hr::Thread(&fun1, "name_" + std::to_string(i)));
        //sylar::Thread::ptr thr2(new sylar::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        //thrs.push_back(thr2);
    }

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    //fun1();
    HR_LOG_INFO(g_logger) << "thread test end";
    HR_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}