
#ifndef LIB_ASIO_REACTOR_UDP_CONNECTOR_H
#define LIB_ASIO_REACTOR_UDP_CONNECTOR_H


#include "socket.h"

#include "../mgr.h"

namespace asio::reactor::udp {

class connector: public simple::reactor::mgr::connector {
    public:
        connector(simple::reactor::mgr* m): connector(static_cast<mgr*>(m)->io_service_) {
        }
        connector(asio::io_service& io_service): simple::reactor::mgr::connector(new socket(io_service)) {
        }

    private:
        socket* get_socket() {
            return static_cast<socket*>(socket_);
        }

    public:
        void connect(const char *host, uint16 port) {
            remote_endpoint_ = asio::ip::udp::endpoint(asio::ip::address::from_string(host), port);
            connect();
        }

    public:
        void connect() {

            get_socket()->close();
            auto self = shared_from_this();

            get_socket()->socket_.open(asio::ip::udp::v4());
            get_socket()->socket_.set_option(asio::ip::udp::socket::reuse_address(true));
            get_socket()->socket_.bind(asio::ip::udp::endpoint());

            connecting();
            get_socket()->socket_.async_connect(remote_endpoint_, [this, self] (std::error_code ec) {
                if (ec)
                    disconnect();
                else {
                    char c = 0;
                    get_socket()->socket_.send(asio::buffer(&c, sizeof(c)));
                    connected();
                }
            });
        }

    private:
        asio::ip::udp::endpoint remote_endpoint_;

};

}

#endif
