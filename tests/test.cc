#include "../sylar/sylar.h"


int main(void) {
    //日志器
    hr::Logger::ptr logger(new hr::Logger);
    ///日志输出地--控制台
    hr::StdoutLogAppender::ptr stdout(new hr::StdoutLogAppender);
    //日志器添加日志输出地
    logger->addAppender(stdout);
    //日志输出地--文件
    hr::FileLogAppender::ptr fileappender(new hr::FileLogAppender("./log.txt"));
    logger->addAppender(fileappender);
    //日志模板
    hr::LogFormatter::ptr fmt(new hr::LogFormatter("%d%T%p%T%m%n"));
    //设置模板
    fileappender->setFormatter(fmt);
    //设置日志等级
    fileappender->setLevel(hr::LogLevel::ERROR);

    //事件
    hr::LogEvent::ptr event(new hr::LogEvent(logger,hr::LogLevel::DEBUG,__FILE__, __LINE__, 0, hr::GetThreadId(), hr::GetFiberId(), time(0),"hr"));
    event->getSS() << "hello sylar log";
    logger->log(hr::LogLevel::ERROR, event);

    HR_LOG_INFO(logger) << "test info";
    HR_LOG_ERROR(logger) << "test error";

    HR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    auto l = hr::LoggerMgr::GetInstance()->getLogger("xx");
    HR_LOG_INFO(l) << "xxx";

    return 0;
}