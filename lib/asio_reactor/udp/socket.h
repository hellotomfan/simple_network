#ifndef LIB_ASIO_REACTOR_UDP_SOCKET_H
#define LIB_ASIO_REACTOR_UDP_SOCKET_H

//#include "acceptor.h"

#include "../../simple/reactor/socket.h"

#include <asio.hpp>

namespace asio::reactor::kcp {
    class connector;
}

namespace asio::reactor::udp {

class socket: public simple::reactor::socket {
    friend class udp::connector;
    friend class kcp::connector;
    public:
        socket(asio::io_service& io_service): socket_(io_service) {
        }
        socket(asio::ip::udp::socket& socket, const std::shared_ptr<asio::ip::udp::endpoint>& endpoint): socket_(std::move(socket)), endpoint_(endpoint) {
        }
    public:
        void open() {
            socket_.non_blocking(true);
            do_read();
            do_write();
        }

        void close() {
            socket_.close();
            io_->on_close();
        }

        void do_read() {
            auto self(io_->shared_from_this());
            size_t space = recv_buff_.get_continguious_space();
            socket_.async_receive(asio::buffer(recv_buff_.get_end(), space), [this, self](std::error_code ec, size_t count) {
                if (ec) {
                    close();
                } else {
                    recv_buff_.written(count);
                    io_->on_read();
                    recv_buff_.reset();
                    do_read();
                }
            });
        }

        void do_write() {
            if (size_t count = send_buff_.get_count()) {
                size_t continguious_count = send_buff_.get_continguious_count();
                std::error_code ec;
                if (count == continguious_count) {
                    socket_.send(asio::buffer(send_buff_.get_start(), count), 0, ec);
                } else {
                    uint8 buff[65536] = {};
                    memcpy(buff, send_buff_.get_start(), continguious_count);
                    send_buff_.remove(continguious_count);
                    continguious_count = send_buff_.get_continguious_count();
                    memcpy(&buff[count - continguious_count], send_buff_.get_start(), continguious_count);
                    socket_.send(asio::buffer(buff, count), 0, ec);
                }
                if (ec) {
                    close();
                }
                send_buff_.reset();
            }
        }

    protected:
        asio::ip::udp::socket socket_;

    protected:
        std::shared_ptr<asio::ip::udp::endpoint> endpoint_;

};



}


#endif
