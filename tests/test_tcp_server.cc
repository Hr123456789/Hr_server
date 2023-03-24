#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"

hr::Logger::ptr g_logger = HR_LOG_ROOT();

void run() {
    auto addr = hr::Address::LookupAny("0.0.0.0:8033");
    //auto addr2 = sylar::UnixAddress::ptr(new sylar::UnixAddress("/tmp/unix_addr"));
    std::vector<hr::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    hr::TcpServer::ptr tcp_server(new hr::TcpServer);
    std::vector<hr::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    
}
int main(int argc, char** argv) {
    hr::IOManager iom(3);
    iom.schedule(run);
    return 0;
}