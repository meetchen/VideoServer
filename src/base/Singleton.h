#pragma once
#include "NoCopyable.h"
#include <pthread.h>

namespace vdse
{
    namespace base
    {
        template<typename T>
        class Singletion : public NoCopyable
        {
            public:
                Singletion() = delete;
                ~Singletion() = delete;
                static T*& Instance()
                {
                    // 确保只被调用一次 饿汉式
                    pthread_once(&ponce_, &Singletion::init);
                    return value_;
                }
            private:
                static void init()
                {
                    if (!value_)
                    {
                        value_ = new T();
                    }
                }
                static pthread_once_t ponce_;
                static T* value_;
        };

        template<typename T>
        pthread_once_t Singletion<T>::ponce_ = PTHREAD_ONCE_INIT;

        template<typename T>
        T* Singletion<T>::value_ = nullptr;
    }
}