
# 流媒体直播系统
### 2024-5-30
- 搭建环境，完成时间相关函数 [TTime.cpp](src/base/TTime.cpp)（获取当前时间，返回字符串格式的时间等）
- 完成部分字符串相关操作函数 [StringUtils.h](src/base/StringUtils.h)
### 2024-6-1
- 完成单例模式相关基类，约束[NoCopyable.h](src/base/NoCopyable.h)等
- 完成定时任务[Task.h](src/base/Task.h)，定时任务管理器[TaskMg.h](src/base/TaskMg.h)等相关逻辑
- 完成日志文件数据流相关逻辑[Logger.h](src/base/Logger.h)
### 2024-6-2
- 完成日志管理类逻辑[FileMg.h](src/base/FileMg.h)
- 完成日志管理功能测试[TestBase.cpp](src/base/TestBase.cpp)
### 2024-6-3
- 完成jsoncpp的编译安装
- 完成对于json配置文件的加载保存[Config.h](src/base/Config.h)
### 2024-6-23
- 开始进行网络库方面函数实现
- 完成对于epoll事件循环的类创建[EventLoop.cpp](src/network/net/EventLoop.cpp)，以及测试
- 处理bug，包括对于event初始化为-1，导致后续“|”加权限，加不上，无法epoll触发读写事件
- 使用future primose等完成线程的同步
### 2024-6-24
- 完成对于Eventloop绑定任务的事件，任务队列 TODO : 待捋清楚
- 完成基于智能指针的时间轮[TimingWheel.h](src/network/net/TimingWheel.h)
    - 使用指针智能析构的时候，调用析构函数，将回调函数放到智能指针的析构函数里面（deque）
    - 使用移动构造函数，避免中间的转换

