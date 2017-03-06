// TickCounter.h

#ifndef _FRAMEWORK_TIMER_TICK_COUNTER_H
#define _FRAMEWORK_TIMER_TICK_COUNTER_H

namespace framework
{
    namespace timer
    {

        class TickCounter
        {
        public:
            typedef boost::uint64_t count_value_type;

            static boost::uint64_t tick_count();

        public:
            TickCounter(
                bool start = true)
                : running_(false)
                , count_(0)
            {
                if (start)
                    this->start();
            }

            void start()
            {
                running_ = true;
                reset();
            }

            void stop()
            {
                running_ = false;
            }

            void reset()
            {
                count_ = tick_count();
            }

            bool running() const
            {
                return running_;
            }

            boost::uint32_t elapsed() const
            {
                return (boost::uint32_t)(tick_count() - count_);
            }

            boost::uint64_t count() const
            {
                return count_;
            }

        private:
            bool running_;
            boost::uint64_t count_;
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_TICK_COUNTER_H
