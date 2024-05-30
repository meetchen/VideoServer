#include "StringUtils.h"

using namespace vdse::base;


bool StringUtils::StartsWith(const string &s, const string &sub)
{
    if (sub.empty()) return true;
    if (s.empty()) return false;
    
    auto len = s.size(), slen = sub.size();

    if (slen > len) return false;

    return s.compare(0, slen, sub) == 0;
}

bool StringUtils::EndsWith(const string &s, const string &sub)
{
    if (sub.empty()) return true;
    if (s.empty()) return false;
    
    auto len = s.size(), slen = sub.size();

    if (slen > len) return false;

    return s.compare(len - slen, slen, sub) == 0;
}


string StringUtils::FilePath(const string &path)
{
    auto pos = path.find_last_of("/\\");
    if (pos != string::npos)
    {
        return path.substr(0, pos);
    }
    return "./";
}

string StringUtils::FileNameExt(const string &path)
{
    auto pos = path.find_last_of("/\\");
    if (pos != string::npos && pos + 1 < path.size())
    {
        return path.substr(pos);
    }
    return path;
}

string StringUtils::FileName(const string &path)
{
    string fileName = FileNameExt(path);
    auto pos = fileName.find_last_of(".");

    if (pos != string::npos && pos != 0)
    {
        return fileName.substr(0, pos);
    }
    return fileName;    
    
}
string StringUtils::Extension(const string &path)
{
    string fileName = FileNameExt(path);
    auto pos = fileName.find_last_of(".");

    // 有. 不是第一个或者最后一个位置
    if (pos != string::npos && pos != 0 && pos + 1 < fileName.size())
    {
        return fileName.substr(pos + 1);
    }
    return string();  
}