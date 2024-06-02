#include "Logger.h"
#include <iostream>

using namespace vdse::base;

void Logger::SetLogLevel(const LogLevel& level)
{
    level_ = level;
}

LogLevel Logger::GetLogLevel()
{
    return level_;
}

void Logger::write(const std::string &str)
{
    std::cout << str;
}