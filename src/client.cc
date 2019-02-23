#include "../lib/asio_reactor/mgr.h"
#include "../lib/asio_reactor/timer/timer.h"
#include "../lib/asio_reactor/tcp/acceptor.h"
#include "../lib/asio_reactor/tcp/connector.h"
#include "../lib/asio_reactor/udp/connector.h"
#include "../lib/asio_reactor/udp/socket.h"
#include "../lib/asio_reactor/kcp/connector.h"
#include "../lib/asio_reactor/kcp/socket.h"
#include "../lib/asio_reactor/kcp/connection.h"

#include "../lib/simple/reactor/connection.h"

#include "../lib/simple/reactor/buffer/circle.h"


#include <iostream>
#include <string.h>

class connector: public asio::reactor::kcp::connector {
    public:
        connector(simple::reactor::mgr* mgr): asio::reactor::kcp::connector(mgr) {
        }
    private:
        void on_recv(simple::reactor::packet::reader& packet) {
            //std::cout << __PRETTY_FUNCTION__ << std::endl;
        } 
        void on_connected() {
            std::cout << __PRETTY_FUNCTION__ << std::endl; //asio::reactor::timer::relay(1.f, true);
        }
        void on_disconnected() {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
        void on_time(uint32 ms) {
            asio::reactor::kcp::connector::on_time(ms);
            //std::cout << __PRETTY_FUNCTION__ << std::endl;
            if (is_disconnected()) {
                connect();
            }
            else if (is_connected()) {
                //std::cout << "send" << std::endl;
                auto packet = get_packet_writer();
                packet << i++;
                send(packet);
            }
        }

    private:
        int i = 0;
};

class timer: public asio::reactor::timer::timer {
    public:
        timer(asio::reactor::mgr *mgr): asio::reactor::timer::timer(mgr) {
            delay(0.1f);
        }

    private:
        void on_time(uint32) {
        }

};

int main() {
    auto &mgr = asio::reactor::mgr::get_instance();
    mgr.connect<connector>("127.0.0.1", 11111);
    mgr.run();
}
