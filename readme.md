<!--
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-30 18:27:36
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-02 16:22:47
 * @FilePath: /VideoServer/readme.md
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
-->

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
- 完成对于Eventloop绑定任务的事件，任务队列 
- 完成基于智能指针的时间轮[TimingWheel.h](src/network/net/TimingWheel.h)
    - 使用指针智能析构的时候，调用析构函数，将回调函数放到智能指针的析构函数里面（deque）
    - 使用移动构造函数，避免中间的转换
### 2024-6-25
- 完成[InetAddress.h](src/network/base/InetAddress.h)即对网络连接中的ip地址端口号，相关的函数进行了封装
- 完成[SocketOpt.h](src/network/base/SocketOpt.h)，即对socket相关操作进行封装，例如建立socket，监听，接受等等
- 完成对于以上两个模块的测试[InetAddressTest.cpp](src/network/net/tests/InetAddressTest.cpp)、[SocketOptTest.cpp](src/network/net/tests/SocketOptTest.cpp)
- 调试bug，少写了个s。。。
### 2024-6-26
- 完成[Connection.h](src/network/net/Connection.h)，继承于Event，可以做为TcpConn等的父类，提供对于链接的管理，激活等操作
- 开始[TcpConnection.h](src/network/net/TcpConnection.h)，太累了，明天再写。
### 2024-6-26
- 完成[SocketOptTest.cpp](src/network/net/tests/SocketOptTest.cpp)，中对于server端相关接口测试
- 完成[Acceptor.h](src/network/net/Acceptor.h)，以及[AcceptorTest.cpp](src/network/net/tests/AcceptorTest.cpp)
### 2024-6-29
- 完成[TcpClient.h](src/network/TcpClient.h)、[TcpServer.h](src/network/TcpServer.h)
- 完成[TcpServerTest.cpp](src/network/net/tests/TcpServerTest.cpp)、[TcpClientTest.cpp](src/network/net/tests/TcpClientTest.cpp)
### 2024-6-30
- 完成[UdpSocket.h](src/network/net/UdpSocket.h)、[UdpClient.h](src/network/UdpClient.h)、[UdpServer.h](src/network/UdpServer.h)
- 完成[UdpServerTest.cpp](src/network/net/tests/UdpServerTest.cpp)、[UdpClientTest.cpp](src/network/net/tests/UdpClientTest.cpp)
- [network](src/network) 完结💐
### 2024-7-1
- [TestContext.h](src/network/TestContext.h),结合TcpConnection完成上下文模块的测试
- 开启rtmp协议实现模块，[MMediaHandler.h](src/mmedia/base/MMediaHandler.h), 多媒体模块的回调类
- [RtmpHandshake](src/mmedia/rtmp/RtmpHandshake.h)，Rtmp简单握手与复杂握手的状态机实现