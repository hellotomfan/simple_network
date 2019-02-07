#ifndef LIB_ASIO_REACTOR_TIMER_H
#define LIB_ASIO_REACTOR_TIMER_H

#include "../simple/reactor/event.h"

#include <asio.hpp>

#include <chrono>

namespace asio::reactor {
    class timer: public simple::reactor::event::timer {
        public:
            timer(simple::reactor::mgr *m): timer(static_cast<mgr*>(m)->io_service_)  {
            }
            timer(asio::io_service& io_service): timer_(io_service) {
            }


        public:
            void relay(f32 seconds, bool repeat = false) {
                uint32 milliseconds = seconds * 1000;
                timer_.expires_from_now(std::chrono::milliseconds(milliseconds));
                do_time(milliseconds, repeat);
            }

            void cancel() {
                timer_.cancel();
            }

        private:

            void do_time(uint32 milliseconds, bool repeat) {
                auto self = this->shared_from_this();
                timer_.async_wait([this, milliseconds, repeat, self] (std::error_code ec) {
                        timer_.expires_at(timer_.expires_at() + std::chrono::milliseconds(milliseconds));
                        on_time();
                        if (repeat) do_time(milliseconds, repeat);
                });
            }

        private:
            asio::high_resolution_timer timer_;

    };
}

#endif
