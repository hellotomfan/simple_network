#ifndef LIB_ASIO_REACTOR_TIMER_BASE_H
#define LIB_ASIO_REACTOR_TIMER_BASE_H

#include "../../simple/reactor/event.h"

#include <asio.hpp>

#include <chrono>

namespace asio::reactor::timer {
    class base: public simple::reactor::event::timer {
        public:
            base(asio::io_service& io_service): timer_(io_service) {
            }


        public:
            void delay(f32 seconds, bool repeat = false) {
                uint32 milliseconds = seconds * 1000;
                timer_.expires_from_now(std::chrono::milliseconds(milliseconds));
                //last_ = std::chrono::steady_clock::now();
                do_time(milliseconds, repeat);
            }

        public:
            void cancel() {
                timer_.cancel();
            }

        private:
            void do_time(uint32 milliseconds, bool repeat) {
                auto self = this->shared_from_this();
                timer_.cancel();
                timer_.async_wait([this, milliseconds, repeat, self] (std::error_code ec) {
                    if (!ec) {
                        //timer_.expires_at(timer_.expires_at() + std::chrono::milliseconds(milliseconds));
                        timer_.expires_from_now(std::chrono::milliseconds(milliseconds));
                        auto now = std::chrono::system_clock::now();
                        //auto elapse_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_).count();
                        on_time(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
                        //last_ += std::chrono::milliseconds(elapse_ms);
                        if (repeat) do_time(milliseconds, repeat);
                    }
                });
            }

        private:
            asio::high_resolution_timer timer_;
            //asio::steady_timer timer_;

        private:
            //std::chrono::system_clock::time_point last_;

    };
}

#endif
