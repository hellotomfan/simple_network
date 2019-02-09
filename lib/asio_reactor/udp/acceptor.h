#ifndef LIB_ASIO_REACTOR_UDP_ACCEPTOR_H
#define LIB_ASIO_REACTOR_UDP_ACCEPTOR_H


#include "../mgr.h"
#include "socket.h"

#include <asio.hpp>

#include <unordered_map>


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

    public:
        bool is_accepted(const asio::ip::udp::endpoint& remote_endpoint) {
            for (auto it = remote_endpoints_.begin(); it != remote_endpoints_.end(); ++it) {
                if (**it == remote_endpoint) {
                    return true;
                }
            }
            return false;
        }

    public:
        void close(const asio::ip::udp::endpoint& remote_endpoint) {
            for (auto it = remote_endpoints_.begin(); it != remote_endpoints_.end(); ++it) {
                if (**it == remote_endpoint) {
                    remote_endpoints_.erase(it);
                    return;
                }
            }
        }

    private:
        void do_accept() {
            auto self = this->shared_from_this();
            socket_.async_receive_from(asio::buffer(buffer_, sizeof(buffer_)), remote_endpoint_, [this, self](std::error_code ec, size_t n) {
                if (!ec) {
                    if (!is_accepted(remote_endpoint_)) {
                        asio::ip::udp::socket socket(get_mgr()->io_service_);
                        socket.open(asio::ip::udp::v4());
                        socket.set_option(asio::ip::udp::socket::reuse_address(true));
                        socket.bind(endpoint_);
                        socket.connect(remote_endpoint_, ec);
                        if (!ec) {
                            std::shared_ptr<asio::ip::udp::endpoint> remote_endpoint(new asio::ip::udp::endpoint(remote_endpoint_), [this, self](asio::ip::udp::endpoint* endpoint) {
                                close(*endpoint);
                                delete endpoint;
                            });
                            remote_endpoints_.push_back(remote_endpoint.get());
                            on_connected(new udp::socket(socket, remote_endpoint));
                        }
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
        std::vector<asio::ip::udp::endpoint*> remote_endpoints_;

    private:
        asio::ip::udp::socket socket_;
};
}

#endif
