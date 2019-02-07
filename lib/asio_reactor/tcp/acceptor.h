#ifndef LIB_ASIO_REACTOR_TCP_ACCEPTOR_H
#define LIB_ASIO_REACTOR_TCP_ACCEPTOR_H

#include "socket.h"
#include "../mgr.h"

#include <asio.hpp>

namespace asio::reactor::tcp {

class acceptor: public simple::reactor::mgr::acceptor {
    public:
        acceptor(simple::reactor::mgr *m): 
            simple::reactor::mgr::acceptor(m),
            socket_(static_cast<mgr*>(m)->io_service_), 
            acceptor_(static_cast<mgr*>(m)->io_service_)  {
        }

    public:
        void listen(const char *host, uint16 port) {
            asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(host), port);
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen();
            do_accept();
        }

    public:
        mgr* get_mgr() {
           return static_cast<mgr*>(m_);
        }

    private:
        void do_accept() {
            auto self = this->shared_from_this();
            acceptor_.async_accept(socket_,
                [this, self](std::error_code ec) {
                if (!ec) {
                    auto socket = new tcp::socket(socket_);
                    on_connected(socket);
                }
                do_accept();
            });
        }


    private:
        asio::ip::tcp::socket socket_;
        asio::ip::tcp::acceptor acceptor_;
};

}


#endif

