#ifndef LIB_ASIO_REACTOR_UDP_ACCEPTOR_H
#define LIB_ASIO_REACTOR_UDP_ACCEPTOR_H


#include "socket.h"
#include "../mgr.h"

#include <asio.hpp>


namespace asio::reactor::udp {

class acceptor: public simple::reactor::mgr::acceptor {
    public:
        acceptor(simple::reactor::mgr *m): 
            simple::reactor::mgr::acceptor(m),
            socket_(static_cast<mgr*>(m)->io_service_) {
        }

    public:
        void listen(const char *host, uint16 port) {
            endpoint_ = asio::ip::udp::endpoint(asio::ip::address::from_string(host), port);
            socket_.open(asio::ip::udp::v4());
            socket_.set_option(asio::ip::udp::socket::reuse_address(true));
            socket_.bind(endpoint_);
            do_accept();
        }

    public:
        mgr* get_mgr() {
           return static_cast<mgr*>(m_);
        }

    private:
        void do_accept() {
            auto self = this->shared_from_this();
            socket_.async_receive_from(asio::buffer(buffer_, sizeof(buffer_)), remote_endpoint_, [this, self](std::error_code ec, size_t n) {
                if (!ec) {
                    asio::ip::udp::socket socket(get_mgr()->io_service_);
                    socket.open(asio::ip::udp::v4());
                    socket.set_option(asio::ip::udp::socket::reuse_address(true));
                    socket.bind(endpoint_);
                    socket.connect(remote_endpoint_, ec);
                    if (!ec) {
                        on_connected(new udp::socket(socket));
                    }
                }
                do_accept();
            });
        }

    private:
        uint8 buffer_[1];

    private:
        asio::ip::udp::endpoint endpoint_;
        asio::ip::udp::endpoint remote_endpoint_;

    private:
        asio::ip::udp::socket socket_;
};

}

#endif
