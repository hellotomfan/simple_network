#ifndef LIB_ASIO_REACTOR_TCP_CONNECTOR_H
#define LIB_ASIO_REACTOR_TCP_CONNECTOR_H


#include "socket.h"

#include "../../simple/reactor/mgr.h"

namespace asio::reactor::tcp {

class connector: public simple::reactor::mgr::connector<> {
    public:
        connector(simple::reactor::mgr* m): connector(static_cast<mgr*>(m)->io_service_) {
        }
        connector(asio::io_service& io_service): simple::reactor::mgr::connector<>(new socket(io_service)) {
        }

    private:
        socket* get_socket() {
            return static_cast<socket*>(socket_);
        }

    public:
        void connect(const char *host, uint16 port) {
            remote_endpoint_ = asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port);
            connect();
        }

    public:
        void connect() {
            get_socket()->close();
            auto self = shared_from_this();
            connecting();
            get_socket()->socket_.async_connect(remote_endpoint_, [this, self] (std::error_code ec) {
                if (!ec)
                    connected();
                else
                    disconnect();
            });
        }

    private:
        asio::ip::tcp::endpoint remote_endpoint_;

};

}


#endif
