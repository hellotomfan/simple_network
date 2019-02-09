#include "../lib/asio_reactor/mgr.h"
#include "../lib/asio_reactor/timer.h"
#include "../lib/asio_reactor/tcp/acceptor.h"
#include "../lib/asio_reactor/tcp/connector.h"
#include "../lib/asio_reactor/udp/connector.h"
#include "../lib/asio_reactor/udp/socket.h"

#include "../lib/simple/reactor/connection.h"

#include "../lib/simple/reactor/buffer/circle.h"


#include <iostream>
#include <string.h>

class connector: public asio::reactor::udp::connector, public asio::reactor::timer  {
    public:
        connector(simple::reactor::mgr* mgr): asio::reactor::udp::connector(mgr), asio::reactor::timer(mgr) {
        }
    private:
        void on_recv(simple::reactor::packet::reader& packet) {
            //std::cout << __PRETTY_FUNCTION__ << std::endl;
        } 
        void on_connected() {
            //std::cout << __PRETTY_FUNCTION__ << std::endl; //asio::reactor::timer::relay(1.f, true);
            asio::reactor::timer::relay(1.f, true);
        }
        void on_disconnected() {
            //std::cout << __PRETTY_FUNCTION__ << std::endl;
            asio::reactor::timer::relay(1.f, true);
        }
        void on_time() {
            //std::cout << __PRETTY_FUNCTION__ << std::endl;
            if (is_disconnected()) {
                connect();
            }
            else if (is_connected()) {
                auto packet = get_packet_writer();
                packet << i++;
                asio::reactor::udp::connector::send(packet);
            }
        }

    private:
        int i = 0;
};


int main() {
    auto &mgr = asio::reactor::mgr::get_instance();
    mgr.connect<connector>("127.0.0.1", 11111);
    mgr.run();
}
