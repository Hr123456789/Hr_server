#include "../sylar/sylar.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

hr::Logger::ptr g_logger = HR_LOG_ROOT();

//0是标准输入
int sock = 0;

void test_fiber() {
    HR_LOG_INFO(g_logger) << "test_fiber sock = "<< sock;

    ////创建监听套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    //初始化客户端地址结构
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family= AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        HR_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        hr::IOManager::GetThis()->addEvent(sock, hr::IOManager::READ, [](){
            HR_LOG_INFO(g_logger) << "read callback";
        });
        hr::IOManager::GetThis()->addEvent(sock, hr::IOManager::WRITE, [](){
            HR_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            hr::IOManager::GetThis()->cancelEvent(sock, hr::IOManager::READ);
            close(sock);
        });
    } else {
        HR_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    //创建就开始调度
    hr::IOManager iom(2, false,"test");
    //添加调度任务
    iom.schedule(&test_fiber);
}

hr::Timer::ptr s_timer;
void test_timer() {
    hr::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        HR_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(2000, true);
            //s_timer->cancel();
        }
    }, true);
}

int main(int argc, char** argv) {
    //test1();
    test_timer();
    return 0;
}