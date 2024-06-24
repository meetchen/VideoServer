#include "network/net/TimingWheel.h"
#include "base/TTime.h"
#include "network/base/Network.h"
#include <iostream>


using namespace vdse::network;


TimingWheel::TimingWheel()
:wheels_(4)
{
    wheels_[kTimingTypeSecond].resize(60);
    wheels_[kTimingTypeMinute].resize(60);
    wheels_[kTimingTypeHour].resize(24);
    wheels_[kTimingTypeDay].resize(30);
}

TimingWheel::~TimingWheel()
{

}

void TimingWheel::InsertEntry(uint32_t delay, EntryPtr entryPtr)
{
    if (delay <= 0) // 小于0秒，立即执行
    {
        entryPtr.reset();
    }
    else if (delay <= kTimingMinute) // 小于一分钟插入到秒的时间轮
    {
        InsertSecondEntry(delay, entryPtr);
    }
    else if (delay <= kTimingHour) // 小于一小时的 插入到 分钟的 时间轮
    {
        InsertMinuteEntry(delay, entryPtr);
    }
    else if (delay <= kTimingDay) // 小于一天的 插入到 小时的 时间轮
    {
        InsertHourEntry(delay, entryPtr);
    }
    else if (delay <= kTimingDay * 30) // 小于30天的 插入到 天 时间轮
    {
        InsertDayEntry(delay, entryPtr);
    }
    else
    {
        NETWORK_ERROR << "can't timing over 30 days !!!";
        std:: cout << "can't timing over 30 days !!!" << std::endl;
        return;
    }
    
}

void TimingWheel::OnTimer(uint64_t now)
{
    if (now - last_ts_ >= 1)
    {
        last_ts_ = now;
        tick_ ++;
        PopUp(wheels_[kTimingTypeSecond]);
        // 计数刚好达到一分钟
        if (tick_ % kTimingMinute == 0)
        {
            PopUp(wheels_[kTimingTypeMinute]);
        }
        if (tick_ % kTimingHour == 0)
        {
            PopUp(wheels_[kTimingTypeHour]);
        }
        if (tick_ % kTimingDay == 0)
        {
            PopUp(wheels_[kTimingTypeDay]);
        }
    }
}

void TimingWheel::PopUp(Wheel &wheel)
{
    WheelEntry we;
    wheel.front().swap(we);
    wheel.pop_front();
    wheel.emplace_back(WheelEntry());
}

void TimingWheel::RunAfter(double delay, const Func &cb)
{
    // EntryPtr 接受一个 CallBackEntryPtr
    // CallBackEntryPtr 析构的时候调用cb
    InsertEntry(delay, std::make_shared<CallBackEntry>(cb));
}

void TimingWheel::RunAfter(double delay, Func &&cb)
{
    InsertEntry(delay, std::make_shared<CallBackEntry>(std::move(cb)));
}

void TimingWheel::RunEvery(double interval, const Func &cb)
{
    InsertEntry(interval, std::make_shared<CallBackEntry>([this, interval, cb](){
        cb();
        RunEvery(interval, cb);
    }));
}

void TimingWheel::RunEvery(double interval, Func &&cb)
{
    InsertEntry(interval, std::make_shared<CallBackEntry>([this, interval, cb](){
        cb();
        RunEvery(interval, std::move(cb));
    }));
}


void TimingWheel::InsertSecondEntry(uint32_t delay, EntryPtr entryPtr)
{
    wheels_[kTimingTypeSecond][delay - 1].emplace(entryPtr);
}

void TimingWheel::InsertMinuteEntry(uint32_t delay, EntryPtr entryPtr)
{
    int second = delay % kTimingMinute;
    int minute = delay / kTimingMinute;
    // 
    auto callBackEntryPtr = std::make_shared<CallBackEntry>([this, second, entryPtr](){
        InsertSecondEntry(second, entryPtr);
    });
    wheels_[kTimingTypeMinute][minute - 1].emplace(callBackEntryPtr);
}

void TimingWheel::InsertHourEntry(uint32_t delay, EntryPtr entryPtr)
{
    int minute = delay % kTimingHour;
    int hour = delay / kTimingHour;

    auto callBackEntryPtr = std::make_shared<CallBackEntry>([this, minute, entryPtr](){
        InsertMinuteEntry(minute, entryPtr);
    });

    wheels_[kTimingTypeHour][hour - 1].emplace(callBackEntryPtr);
}


void TimingWheel::InsertDayEntry(uint32_t delay, EntryPtr entryPtr)
{
    int hour = delay % kTimingDay;
    int day = delay / kTimingDay;

    auto callBackEntryPtr = std::make_shared<CallBackEntry>([this, hour, entryPtr](){
        InsertHourEntry(hour, entryPtr);
    });

    wheels_[kTimingTypeDay][day - 1].emplace(callBackEntryPtr);

}