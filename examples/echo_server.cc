#include "./sylar/tcp_server.cc"
#include "./sylar/log.h"
#include "./sylar/iomanager.h"
#include "./sylar/bytearray.h"
#include "./sylar/address.h"

static hr::Logger::ptr g_logger = HR_LOG_ROOT();

class EchoServer : public hr::TcpServer {
public:
    EchoServer(int type);
    void handleClient(hr::Socket::ptr client);
private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type) {
}

void EchoServer::handleClient(hr::Socket::ptr client) {
    HR_LOG_INFO(g_logger) << "handleClient" << *client;
    hr::ByteArray::ptr ba(new hr::ByteArray);
    while(true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        //读取数据
        int rt = client->recv(&iovs[0], iovs.size());
        std::cout<<"recv"<<rt<<std::endl;
        //客户端断开
        if(rt == 0) {
            HR_LOG_INFO(g_logger) << "client close :" << *client;
            break;
        } else if(rt < 0) {
            HR_LOG_INFO(g_logger) << "client error rt =" << rt
                << "errno=" << errno << "errstr="<<strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        if(m_type == 1) {
            std::cout << ba->toString();
        } else {
            std::cout << ba->toHexString();
        }
        std::cout.flush();
    }
}

int type = 1;

void run() {
    HR_LOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es(new EchoServer(type));
    auto addr = hr::Address::LookupAny("0.0.0.0:8033");
    while(!es->bind(addr)) {
        sleep(2);
        std::cout<<"1"<<std::endl;
    }
    es->start();
}

int main(int argc, char**argv) {
    if(argc < 2) {
        HR_LOG_INFO(g_logger) << "used as[" << argv[0] << "-t] or [" << argv[0] << "-b]";
    }

    if(!strcmp(argv[1], "-b")) {
        type = 2;
    }

    hr::IOManager iom(3);
    iom.schedule(run);
    return 0;
}