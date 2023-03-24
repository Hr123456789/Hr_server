#include "sylar/http/http_server.h"
#include "sylar/log.h"

static hr::Logger::ptr g_logger = HR_LOG_ROOT();

#define XX(...) #__VA_ARGS__

hr::IOManager::ptr worker;

void run() {
    g_logger->setLevel(hr::LogLevel::INFO);
    hr::http::HttpServer::ptr server(new hr::http::HttpServer(true));
    hr::Address::ptr addr =  hr::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx", [](hr::http::HttpRequest::ptr req
                ,hr::http::HttpResponse::ptr rsp
                ,hr::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/sylar/*", [](hr::http::HttpRequest::ptr req
                ,hr::http::HttpResponse::ptr rsp
                ,hr::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });

    sd->addGlobServlet("/sylarx/*", [](hr::http::HttpRequest::ptr req
                ,hr::http::HttpResponse::ptr rsp
                ,hr::http::HttpSession::ptr session) {
            rsp->setBody(XX(<html>
<head><title>404 Not Found</title></head>
<body>
<center><h1>404 Not Found</h1></center>
<hr><center>nginx/1.16.0</center>
</body>
</html>
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
));
            return 0;
    });

    server->start();
}

int main(int argc, char** argv) {
    hr::IOManager iom(1);
    //worker.reset(new hr::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}