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

/*
class connector: public asio::reactor::tcp::connector, public asio::reactor::timer {
    public:
        connector(simple::reactor::mgr* mgr): asio::reactor::tcp::connector(mgr), asio::reactor::timer(mgr) {
            send_value_ = recv_value_ = 0;
        }
        ~connector() {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
    private:
        void on_time() {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            connect();
        }
        void on_connected() {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            send();
        }
        void on_disconnected() {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            asio::reactor::timer::relay(1.f, false);
            send_value_ = recv_value_ = 0;
        }
        void on_recv(simple::reactor::packet::reader& packet) {
            uint32 recv_value;
            packet >> recv_value;
            //std::cout << ">> "<< recv_value << std::endl;
            assert(recv_value = recv_value_ + 1);
            recv_value_ = recv_value;
            send();
        }
        void send() {
            auto count = (rand() % 17800) + 1;
            auto pending_count = send_value_ - recv_value_;
            if (pending_count < count) {
                for (int i = 0; i < count - pending_count; ++i) {
                    auto packet = get_packet_writer();
                    packet << ++send_value_;
                    asio::reactor::tcp::connector::send(packet);
                    //std::cout << "<< " << send_value_ << std::endl;
                }
            }
        }

    private:
        uint32 send_value_;
        uint32 recv_value_;
};
*/

class connector: public asio::reactor::udp::connector, public asio::reactor::timer  {
    public:
        connector(simple::reactor::mgr* mgr): asio::reactor::udp::connector(mgr), asio::reactor::timer(mgr) {
        }
    private:
        void on_recv(simple::reactor::packet::reader& packet) {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        } 
        void on_connected() {
            asio::reactor::timer::relay(1.f, true);
        }
        void on_time() {
            auto packet = get_packet_writer();
            packet << i++;
            asio::reactor::udp::connector::send(packet);
        }

    private:
        int i = 0;
};


int main() {
    auto &mgr = asio::reactor::mgr::get_instance();
    mgr.connect<connector>("127.0.0.1", 11111);
    mgr.run();
}
