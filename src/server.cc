#include "../lib/asio_reactor/mgr.h"
#include "../lib/asio_reactor/tcp/acceptor.h"
#include "../lib/asio_reactor/tcp/connector.h"
#include "../lib/asio_reactor/timer.h"

#include "../lib/simple/reactor/connection.h"

#include "../lib/asio_reactor/udp/acceptor.h"

#include <iostream>
#include <set>

class connection: public simple::reactor::connection {
    public:
        connection(simple::reactor::socket* socket): simple::reactor::connection(socket) {
        }
    private:
        void on_recv(simple::reactor::packet::reader& packet) {
            int i;
            packet >> i;
            std::cout << i << std::endl;
            auto out_packet = get_packet_writer();
            out_packet << i;
            send(out_packet);
        }
};


/*
class acceptor: public asio::reactor::tcp::acceptor {
    public:
        acceptor(simple::reactor::mgr *mgr): asio::reactor::tcp::acceptor(mgr) {
        }
    private:
        void on_connected(simple::reactor::socket *socket) {
            auto c = std::shared_ptr<connection>(new connection(socket));
            c->connected();
        }
};
class acceptor: public asio::reactor::kcp::acceptor, public asio::reactor::timer {
    public:
        acceptor(simple::reactor::mgr *mgr): asio::reactor::kcp::acceptor(mgr), asio::reactor::timer(mgr){
            asio::reactor::timer::relay(1.f, true);
        }
    private:
        void on_connected(simple::reactor::socket *socket) {
            c_ = std::shared_ptr<connection>(new connection(socket));
            c_->connected();
        }

    private:
        void on_time() {
            auto packet = c_->get_packet_writer();
            packet << 1;
            c_->send(packet);
        }

    private:
        std::shared_ptr<connection> c_;
};

*/

class acceptor: public asio::reactor::udp::acceptor {
    public:
        acceptor(simple::reactor::mgr *mgr): asio::reactor::udp::acceptor(mgr) {
        }
    private:
        void on_connected(simple::reactor::socket *socket) {
            auto client = std::shared_ptr<connection>(new connection(socket));
            clients_.insert(client);
            client->connected();
        }

    private:
        void on_time() {
            for (auto it = clients_.begin(); it != clients_.end(); ++it) {
                auto client = *it;
                auto packet = client->get_packet_writer();
                packet << 1;
                client->send(packet);
            }
        }

    private:
        std::set<std::shared_ptr<connection>> clients_;
};

int main() {
    auto &mgr = asio::reactor::mgr::get_instance();
    mgr.listen<acceptor>("127.0.0.1", 11111);
    mgr.run();
}
