#include "LogStream.h"
#include "TTime.h"
#include "StringUtils.h"
#include <cstring>
#include <thread>
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>

using namespace vdse::base;


static thread_local pid_t thread_id = 0;

LoggerPtr vdse::base::g_logger = nullptr;

const char *log_str[] = 
{
    " Trace ",
    " Debug ",
    " Info  ",
    " Warn  ",
    " Error "
};

LogStream::LogStream(LoggerPtr &loger, const char *file, int line, LogLevel l, const char *func)
:logger_(loger)
{
    const char *file_name = strrchr(file, '/');
    if (file_name)
    {
        file_name = file_name + 1;
    }
    else
    {
        file_name = file;
    }

    stream_ << TTime::ISOTime() << " ";

    if (!thread_id)
    {
        thread_id = static_cast<pid_t>(::syscall(SYS_gettid));
    }

    stream_ << thread_id;

    stream_ << log_str[l];

    stream_ << " [" << file_name << ":" << line << "] ";

    if (func)
    {
        stream_ << " [" << func << "] ";
    }

}

LogStream::~LogStream()
{
    stream_ << "\n";
    if (logger_)
    {
        logger_->write(stream_.str());
    }
    else
    {
        std::cout << stream_.str();
    }
}