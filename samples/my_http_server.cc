#include "sylar/http/http_server.h"
#include "sylar/log.h"

hr::Logger::ptr g_logger = HR_LOG_ROOT();
hr::IOManager::ptr worker;
void run() {
    g_logger->setLevel(hr::LogLevel::INFO);
    hr::Address::ptr addr = hr::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr) {
        HR_LOG_ERROR(g_logger) << "get address error";
        return;
    }
    //hr::http::HttpServer::ptr http_server(new hr::http::HttpServer(true, worker.get()));
    hr::http::HttpServer::ptr http_server(new hr::http::HttpServer(true));
    bool ssl = false;
    while(!http_server->bind(addr, ssl)) {
        HR_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }

    // if(ssl) {
    //     //http_server->loadCertificates("/home/apps/soft/hr/keys/server.crt", "/home/apps/soft/hr/keys/server.key");
    // }

    http_server->start();
}

int main(int argc, char** argv) {
    hr::IOManager iom(1);
    //worker.reset(new hr::IOManager(4, false));
    iom.schedule(run);
    std::cout<<"start"<<std::endl;
    //当使用usecaller线程进行调度时，在调度器析构函数里调用stop方法才是正式开始调度
    return 0;
}
