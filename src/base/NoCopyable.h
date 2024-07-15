#pragma once

namespace vdse
{
    namespace base
    {   
        // 继承当前类，即不可进行赋值 复制
        class NoCopyable
        {
            protected:
                NoCopyable(){}
                ~NoCopyable(){}
                // 删除赋值
                NoCopyable& operator=(const NoCopyable&) = delete;
                // 删除拷贝构造
                NoCopyable(const NoCopyable&) = delete;
        };    
    }
} 
