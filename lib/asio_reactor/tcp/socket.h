#ifndef LIB_ASIO_REACTOR_TCP_SOCKET_H
#define LIB_ASIO_REACTOR_TCP_SOCKET_H


#include "../../simple/reactor/socket.h"

#include <asio.hpp>

namespace asio::reactor::tcp {

class socket: public simple::reactor::socket {
    friend class connector;

    enum {
        SENDING     = 1 << 0,
        RECEIVING   = 1 << 1,
    };

    public:
        socket(asio::io_service& io_service): state_(0), socket_(io_service) {
        }
        socket(asio::ip::tcp::socket& socket): state_(0), socket_(std::move(socket)) {
        }

    public:
        void open() {
            socket_.non_blocking(true);
            do_read();
            do_write();
        }

        void close() {
            state_ = 0;
            recv_buff_.reset();
            send_buff_.reset();
            socket_.close();
            io_->on_close();
        }

    public:
        void do_read() {
            auto self(io_->shared_from_this());
            socket_.async_wait(asio::ip::tcp::socket::wait_read, [this, self](std::error_code ec) {
                if (ec) {
                    close();
                    return;
                }
                while (uint32 size = recv_buff_.get_continguious_space()) {
                    size_t count = socket_.receive(asio::buffer(recv_buff_.get_end(), size), 0, ec);
                    if (count > 0) {
                        recv_buff_.written(count);
                        io_->on_read();
                    } 
                    if (ec) {
                        if (ec != asio::error::would_block) {
                            close();
                            return;
                        }
                        break;
                    }
                };
                do_read();
            });
        }

        void do_write() {
            if (~state_ & SENDING) {
                if (send_buff_.get_count() > 0) {
                    state_ |= SENDING;
                    auto self(io_->shared_from_this());
                    socket_.async_wait(asio::ip::tcp::socket::wait_write, [this, self](std::error_code ec) {
                        if (ec) {
                            close();
                            return;
                        }
                        do {
                            size_t count = socket_.send(asio::buffer(send_buff_.get_start(), send_buff_.get_continguious_count()), 0, ec);
                            if (count > 0) {
                                send_buff_.remove(count);
                            } 
                            if (ec) {
                                if (ec != asio::error::would_block) {
                                    close();
                                    return;
                                }
                                break;
                            }
                        } while (send_buff_.get_count() > 0);
                        state_ &= ~SENDING;
                        do_write();
                    });
                }
            }
        }

    private:
        uint8 state_;

    private:
        asio::ip::tcp::socket socket_;
};

}

#endif
