#ifndef LIB_ASIO_NETWORK_MGR_H
#define LIB_ASIO_NETWORK_MGR_H


#include "timer/base.h"


#include "../simple/reactor/mgr.h"
#include "../simple/reactor/event.h"

#include <asio.hpp>

#include <signal.h>


namespace asio::reactor {
    namespace tcp {
        class socket;
        class acceptor;
        class connector;
    }
    namespace udp {
        class socket;
        class acceptor;
        class connector;
    }
    namespace kcp {
        class socket;
        class acceptor;
        class connector;
    }

    namespace timer {
        class timer;
    }

class mgr: public simple::reactor::mgr {

    friend class timer::timer;
    friend class tcp::socket;
    friend class tcp::acceptor;
    friend class tcp::connector;
    friend class udp::socket;
    friend class udp::acceptor;
    friend class udp::connector;
    friend class kcp::socket;
    friend class kcp::acceptor;
    friend class kcp::connector;

    class timer: public asio::reactor::timer::base {
        public:
            timer(mgr *m): base(m->io_service_), mgr_(m) {
            }

        public:
            void delay(double time, bool repeat) {
                base::delay(time, repeat);
            }

        private:
            void on_time(uint32 current) {
                mgr_->on_time(current);
            }
        public:
            mgr *mgr_;
    };

    public:
        mgr(): timer_(new timer(this)) {

        }
        ~mgr() {
            timer_.reset();
            timers_.clear();
        }


    public:
        void run() {
            signal(SIGINT, [](int) {
                mgr::get_instance().stop();
            });
            timer_->delay(0.02f, true);
            io_service_.run();
        }

        void stop() {
            io_service_.stop();
        }
        bool stopped() {
            return io_service_.stopped();
        }

        void on_time(uint32 current) {
            auto it = timers_.begin();
            while (it != timers_.end()) {
                auto i = it++;
                (*i)->on_time(current);
            }
        }

    public:
        static mgr& get_instance() {
            static mgr s_mgr;
            return s_mgr;
        }

    private:
        void add(simple::reactor::event::timer *timer) {
            timers_.insert(timer);
        }

        void remove(simple::reactor::event::timer *timer) {
            timers_.erase(timer);
        }


    protected:
        std::unordered_set<simple::reactor::event::timer *> timers_;

    private:
        asio::io_service io_service_;

    private:
        std::shared_ptr<timer> timer_;

};

}

#endif
