#pragma once

#include <vector>
#include <unordered_set>
#include <deque>
#include <memory>
#include <functional>

namespace vdse
{
    namespace network
    {
        using Func = std::function<void()>;

        class CallBackEntry
        {
        public:
            CallBackEntry(const Func &f) : cb_(f)
            {
            }
            // 没有移动构造函数的情况下，传入Lambda表达式时会通过隐式转换为 std::function<void()>，并使用 const std::function<void()> & 构造函数进行初始化。
            // 会多性能开销
            CallBackEntry(Func &&f) : cb_(std::move(f))
            {
            }
            ~CallBackEntry()
            {
                if (cb_)
                {
                    cb_();
                }
            }

        private:
            Func cb_;
        };

        using CallBackEntryPtr = std::shared_ptr<CallBackEntry>;

        // 保存一个CallBackEntry，在需要执行的时候，使用reset函数，释放内存，进行析构
        // 析构CallBackEntry 调用callback
        using EntryPtr = std::shared_ptr<void>;

        // 作为一个时间轮中的一个片，例如以妙为单位中的，第三秒的所有回调
        using WheelEntry = std::unordered_set<EntryPtr>;

        // 使用一个双端队列，保存一个时间轮中的所有片，以秒为单位，即保存60个时间片的入口，
        // 每次到期的时候，队头先到期，释放队头，析构回调
        // 在末尾在补一个空的whellentry，作为第60秒，其余的往前移，当做减少一秒
        using Wheel = std::deque<WheelEntry>;

        // 长度为4,分别保存，秒、分、小时、天的时间轮
        using Wheels = std::vector<Wheel>;


        class TimingWheel
        {
        public:
            TimingWheel();
            ~TimingWheel();

            void InsertEntry(uint32_t delay, EntryPtr entryPtr);
            void OnTimer(uint64_t now);
            void PopUp(Wheel &wheel);

            void RunAfter(double delay, const Func &cb);
            void RunAfter(double delay, Func &&cb);
            void RunEvery(double interval, const Func &cb);
            void RunEvery(double interval, Func &&cb);

        private:
            void InsertSecondEntry(uint32_t delay, EntryPtr entryPtr);
            void InsertMinuteEntry(uint32_t delay, EntryPtr entryPtr);
            void InsertHourEntry(uint32_t delay, EntryPtr entryPtr);
            void InsertDayEntry(uint32_t delay, EntryPtr entryPtr);

            Wheels wheels_;
            int64_t last_ts_{0};
            uint64_t tick_{0};
        };

        enum TimingType 
        {
            kTimingTypeSecond = 0,
            kTimingTypeMinute = 1,
            kTimingTypeHour = 2,
            kTimingTypeDay = 3
        };

        const int kTimingMinute = 60;
        const int kTimingHour = 60*60;
        const int kTimingDay = 60*60*24;
    }

}