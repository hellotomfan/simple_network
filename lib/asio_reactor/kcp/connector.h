
#ifndef LIB_ASIO_REACTOR_KCP_CONNECTOR_H
#define LIB_ASIO_REACTOR_KCP_CONNECTOR_H


#include "socket.h"
#include "connection.h"

#include "../udp/connector.h"

#include "../mgr.h"

#include <ikcp.h>

namespace asio::reactor::kcp {

class connector: public simple::reactor::mgr::connector<connection>, public simple::reactor::event::timer {
    public:
        connector(simple::reactor::mgr* m): connector(static_cast<mgr*>(m)->io_service_) {
        }
        connector(asio::io_service& io_service): connector(new socket(io_service)) {
        }
        connector(socket *socket): simple::reactor::mgr::connector<connection>(socket) {
            asio::reactor::mgr::get_instance().add(this);
        }
        ~connector() {
            asio::reactor::mgr::get_instance().remove(this);
        }

    public:
        void connect(const char *host, uint16 port) {
            remote_endpoint_ = asio::ip::udp::endpoint(asio::ip::address::from_string(host), port);
            connect();
        }

    public:
        void connect() {
            get_socket()->close();

            get_socket()->socket_.open(asio::ip::udp::v4());
            get_socket()->socket_.non_blocking(true);
            get_socket()->socket_.set_option(asio::ip::udp::socket::reuse_address(true));
            get_socket()->socket_.bind(asio::ip::udp::endpoint());

            connecting();
            std::error_code ec;
            get_socket()->socket_.connect(remote_endpoint_, ec);
            if (ec)
                disconnect();
            else {
                uint32 conv = 0;
                get_socket()->socket_.send(asio::buffer(&conv, sizeof(conv)), 0, ec);
                if (!ec)
                    connected();
                else
                    disconnect();
            }
        }

    protected:
        socket* get_socket() {
            return static_cast<socket*>(socket_);
        }

    protected:
        void on_time(uint32 current) {
            get_socket()->update(current);
        }

    private:
        asio::ip::udp::endpoint remote_endpoint_;

};

}

#endif
