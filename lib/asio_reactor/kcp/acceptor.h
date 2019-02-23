#ifndef LIB_ASIO_REACTOR_KCP_ACCEPTOR_H
#define LIB_ASIO_REACTOR_KCP_ACCEPTOR_H


#include "socket.h"
#include "../udp/acceptor.h"

#include "../mgr.h"
#include "../timer/timer.h"

#include <asio.hpp>

#include <unordered_map>


namespace asio::reactor::kcp {

class acceptor: public udp::acceptor, public simple::reactor::event::timer {

    public:
        acceptor(simple::reactor::mgr *m): udp::acceptor(m) {
            asio::reactor::mgr::get_instance().add(this);
        }
        ~acceptor() {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            asio::reactor::mgr::get_instance().remove(this);
        }

    public:
        void listen(const char *host, uint16 port) {
            udp::acceptor::listen(host, port);
            do_accept();
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
                            uint32 conv = get_conv();
                            std::shared_ptr<asio::ip::udp::endpoint> remote_endpoint(new asio::ip::udp::endpoint(remote_endpoint_), [this, conv, self](asio::ip::udp::endpoint* endpoint) {
                                close(conv, *endpoint);
                                delete endpoint;
                            });
                            remote_endpoints_.push_back(remote_endpoint.get());
                            auto so = new kcp::socket(socket, remote_endpoint);
                            std::cout << "create conv:" << conv << std::endl;
                            convs_[conv] = so;
                            auto kcp = ikcp_create(conv, so);
                            ikcp_wndsize(kcp, 128, 128);
                            ikcp_nodelay(kcp, 0, 10, 0, 0);
                            so->set_kcp(kcp);
                            on_connected(so);
                        }
                    }
                    do_accept();
                }
            });

        }

    private:
        void on_time(uint32 current) {
            auto it = convs_.begin();
            while (it != convs_.end()) {
                auto i = it ++;
                i->second->update(current);
            }
        }

    private:
        void close(uint32 conv, const asio::ip::udp::endpoint& remote_endpoint) {
            convs_.erase(conv);
            udp::acceptor::close(remote_endpoint);
        }

    private:
        uint32 get_conv() {
            static uint32 conv = 0;
            conv = ++conv ? conv : 1;
            if (convs_.find(conv) != convs_.end())
                return get_conv();
            return conv;
        }

    private:
        std::unordered_map<uint32, kcp::socket*> convs_;

};
}

#endif
