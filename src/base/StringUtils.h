#pragma once

#include <string>
#include <vector>

namespace vdse
{
    namespace base
    {
        using std::string;
        class StringUtils
        {
            public:
                static bool StartsWith(const string &s, const string &sub);
                static bool EndsWith(const string &s, const string &sub);
                // 返回文件的路径
                static string FilePath(const string &path);
                // 返回文件名以及后缀
                static string FileNameExt(const string &path);
                // 仅返回文件名
                static string FileName(const string &path);
                // 返回后缀
                static string Extension(const string &path);
                // 字符串分割
                static std::vector<string> SplitString(const string &s, const string &delimiter);
        };
    }
}