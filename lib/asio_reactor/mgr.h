#ifndef LIB_ASIO_NETWORK_MGR_H
#define LIB_ASIO_NETWORK_MGR_H

#include "../simple/reactor/mgr.h"

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

class mgr: public simple::reactor::mgr {

    friend class timer;
    friend class tcp::socket;
    friend class tcp::acceptor;
    friend class tcp::connector;
    friend class udp::socket;
    friend class udp::acceptor;
    friend class udp::connector;

    public:
        void run() {
            signal(SIGINT, [](int) {
                    mgr::get_instance().stop();
            });
            io_service_.run();
        }

        void stop() {
            io_service_.stop();
        }

    public:
        static mgr& get_instance() {
            static mgr s_mgr;
            return s_mgr;
        }

    private:
        asio::io_service io_service_;
};

}

#endif
