#include "../lib/asio_reactor/mgr.h"
#include "../lib/asio_reactor/tcp/acceptor.h"
#include "../lib/asio_reactor/tcp/connector.h"
#include "../lib/asio_reactor/timer.h"

#include "../lib/simple/reactor/connection.h"

#include "../lib/asio_reactor/udp/acceptor.h"

#include <iostream>
#include <set>

class connection: public simple::reactor::connection, public asio::reactor::timer {
    public:
        connection(simple::reactor::socket* socket, simple::reactor::mgr *mgr): simple::reactor::connection(socket), asio::reactor::timer(mgr) {
        }

    public:
        void on_connected() {
            asio::reactor::timer::relay(1.f, true);
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

        void on_time() {
            auto packet = get_packet_writer();
            packet << 1;
            send(packet);
        }
};


class acceptor: public asio::reactor::udp::acceptor {
    public:
        acceptor(simple::reactor::mgr *mgr): asio::reactor::udp::acceptor(mgr) {
        }
    private:
        void on_connected(simple::reactor::socket *socket) {
            auto client = std::shared_ptr<connection>(new connection(socket, get_mgr()));
            client->connected();
        }
};

int main() {
    auto &mgr = asio::reactor::mgr::get_instance();
    mgr.listen<acceptor>("127.0.0.1", 11111);
    mgr.run();
}
