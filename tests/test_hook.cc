#include "../sylar/hook.h"
#include "../sylar/sylar.h"
#include "../sylar/iomanager.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

hr::Logger::ptr g_logger = HR_LOG_ROOT();

void test_sleep() {
    hr::IOManager iom(1);
    iom.schedule([]() {
        sleep(2);
        HR_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(1);
        HR_LOG_INFO(g_logger) << "sleep 3";
    });

    HR_LOG_INFO(g_logger) << "test_sleep";
}

// void test_sock() {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);

//     sockaddr_in addr;
//     memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(80);
//     inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
// }

int main(int argc, char** argv) {
    hr::set_hook_enable(true);
    test_sleep();
    return 0;
}