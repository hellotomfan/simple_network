#ifndef LIB_REACTOR_CONNECTION_H
#define LIB_REACTOR_CONNECTION_H

#include "socket.h"
#include "packet.h"
#include "event.h"

#include <iostream>

namespace simple::reactor {

class connection: public event::io {
    
    enum {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
    };

    public:
        connection(socket *socket): state_(DISCONNECTED), socket_(socket) {
            socket_->io_ = this;
        }
        ~connection() {
            delete socket_;
        }

    public:
        bool is_connected() {
            return state_ == CONNECTED;
        }
        bool is_connecting() {
            return state_ == CONNECTING;
        }
        bool is_disconnected() {
            return state_ == DISCONNECTED;
        }

    public:
        void connected() {
            if (state_ != CONNECTED) {
                state_ = CONNECTED;
                on_connected();
                socket_->open();
            }
        }
        void connecting() {
            state_ = CONNECTING;
        }
        void disconnect() {
            if (state_ == DISCONNECTED)
                return;
            state_ = DISCONNECTED;
            socket_->close();
            on_disconnected();
        }

    public:
        packet::writer get_packet_writer() {
            return packet::writer(socket_->send_buff_);
        }

    public:
        void send(packet::reader& packet) {
            auto out_packet = packet::writer(socket_->send_buff_);
            out_packet << packet;
            send(out_packet);
        }
        void send(packet::writer& packet) {
            if (packet.buffer_ == socket_->send_buff_) {
                packet.flush();
                socket_->do_write();
            }
        }

    private:
        void on_read() {
            auto &recv_buff = socket_->recv_buff_;
            do {
                uint32 length;
                if (recv_buff.get_count() < sizeof(length))
                    break;
                recv_buff.read((uint8*)&length, sizeof(length), 0);
                uint32 size = length + sizeof(uint32);
                if (size > recv_buff.get_count())
                    break;
                packet::reader reader(recv_buff);
                on_recv(reader);
                recv_buff.remove(size);
            } while (recv_buff.get_count());
        }

        void on_write() {
        }

        void on_close() {
            if (state_ == CONNECTED) {
                state_ = DISCONNECTED;
                on_disconnected();
            }
        }

    private:
        virtual void on_connected() {}
        virtual void on_disconnected() {}

    private:
        virtual void on_recv(packet::reader&) = 0;

    private:
        uint8 state_;

    protected:
        socket* socket_;
};

}

#endif
