#ifndef LIB_ASIO_REACTOR_KCP_SOCKET_H
#define LIB_ASIO_REACTOR_KCP_SOCKET_H

#include "../udp/acceptor.h"

#include "../../simple/reactor/socket.h"

#include <asio.hpp>
#include <ikcp.h>

namespace asio::reactor::kcp {

class socket: public udp::socket {
    friend class kcp::acceptor;
    friend class kcp::connector;
    public:
        socket(asio::io_service& io_service, ikcpcb *kcp = nullptr): udp::socket(io_service), kcp_(kcp) {
        }
        socket(asio::ip::udp::socket& socket, const std::shared_ptr<asio::ip::udp::endpoint>& endpoint, ikcpcb *kcp = nullptr): udp::socket(socket, endpoint), kcp_(kcp) {
        }

    private:
        void set_kcp(ikcpcb *kcp) {
            if (!kcp_) {
                kcp_ = kcp;
            }
        }

    private:
        void update(uint32 current) {
            if (kcp_) {
                ikcp_update(kcp_, current);
                uint32 size = recv_buff_.get_continguious_space();
                auto count = ikcp_recv(kcp_, (char *)recv_buff_.get_end(), size);
                if (count > 0) {
                    recv_buff_.written(count);
                    io_->on_read();
                }
            }
        }


    private:
        static int output(const char *buf, int len, ikcpcb *kcp, void *user) {
            socket *s = (socket *)user;
            std::error_code ec;
            auto n = s->socket_.send(asio::buffer(buf, len), 0, ec);
            if (ec) 
                s->close();
            return n;
        }

    private:
        void open() {
            if (kcp_) {
                kcp_->output = output;
            }
            socket_.non_blocking(true);
            do_read();
            do_write();
        }

        void close() {
            socket_.close();
            io_->on_close();
            if (kcp_) {
                ikcp_release(kcp_);
                kcp_ = nullptr;
            }
        }

    private:
        void do_read() {
            auto self(io_->shared_from_this());
            socket_.async_receive(asio::buffer(buffer_, sizeof(buffer_)), [this, self](std::error_code ec, size_t count) {
                if (ec) {
                    close();
                } else {
                    uint32 conv = *(uint32*)buffer_;
                    if (!conv)
                        return;
                    if (!kcp_) {
                        kcp_ = ikcp_create(conv, this);
                        ikcp_wndsize(kcp_, 128, 128);
                        ikcp_nodelay(kcp_, 0, 10, 0, 0);
                        kcp_->output = output;
                        do_write();
                    } 
                    //std::cout << "conv:" << kcp_->conv << std::endl;
                    ikcp_input(kcp_, (const char *)buffer_, count);
                    do_read();
                }
            });
        }
        void do_write() {
            if (kcp_ && send_buff_.get_count() > 0) {
                auto count = send_buff_.get_continguious_count();
                ikcp_send(kcp_, (const char *)send_buff_.get_start(), count);
                send_buff_.remove(count);
                do_write();
            }
        }

    private:
        uint8 *buffer_[65535];

    private:
        ikcpcb *kcp_ = nullptr;
};



}


#endif
