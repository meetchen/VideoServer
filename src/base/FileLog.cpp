#include "FileLog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using namespace vdse::base;


bool FileLog::Open(const std::string &filePath)
{
    file_path_ = filePath;
    int fd = open(filePath.c_str(), O_CREAT | O_APPEND | O_WRONLY, DEFFILEMODE);
    if (fd < 0)
    {
        std::cout << "file log open error , filepath : " << filePath << std::endl;
        return false;
    }
    
    fd_ = fd;

    return true;

}
size_t FileLog::WriteLog(const std::string &msg)
{
    int fd = fd_ == -1 ? 1 : fd_;
    return ::write(fd, msg.data(), msg.size());
}

void FileLog::Rotate(const std::string &file)
{
    if (file_path_.empty()) 
    {
        return;
    }

    int fd = ::open(file.c_str(), O_CREAT | O_APPEND | O_WRONLY, DEFFILEMODE);

    if (fd < 0)
    {
        std::cout << "open file log error, path: " << file << std::endl;
        return ;
    }

    ::dup2(fd, fd_);
    close(fd);

}

void FileLog::SetRotate(RotateType type)
{
    rotate_ = type;
}

RotateType FileLog::getRotateType()
{
    return rotate_;
}

int64_t FileLog::FileSize()
{
    return ::lseek64(fd_, 0, SEEK_END);
}

std::string FileLog::FilePath()
{
    return file_path_;
}