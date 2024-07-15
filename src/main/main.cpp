/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-04 11:03:43
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-09 14:46:05
 * @FilePath: /VideoServer/src/main/main.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "base/Config.h"
#include "base/LogStream.h"
#include "base/FileMg.h"
#include "base/TaskMg.h"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <memory>
#include <filesystem>

using namespace vdse::base;

int main(int argc,const char ** argv)
{
    g_logger.reset(new Logger(nullptr));

    g_logger->SetLogLevel(kTrace);

    if(!sConfigMg->LoadConfig("../config/config.json"))
    {
        std::cerr << "load config file failed." << std::endl;
        return -1;
    }
    ConfigPtr config = sConfigMg->GetConfig();
    LogInfoPtr log_info = config->GetLogInfo();
    std::cout << "log level:" << log_info->level 
        << " path:" << log_info->path 
        << " name:" << log_info->name 
        << std::endl;
    
    FileLogPtr log = sFileMg->GetFileLog(log_info->path+log_info->name);
    
    if(!log)
    {
        std::cerr << "log can't open.exit." << std::endl;
        return -1;
    }
    log->SetRotate(log_info->rotate);



    TaskPtr task4 = std::make_shared<Task>([](const TaskPtr &task)
                                        {
                                            sFileMg->OnCheck();
                                            task->Restart();
                                        },
                                        1000);      
    sTaskMg->Add(task4); 
    while(1)
    {
        sTaskMg->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }    
    return 0;
}