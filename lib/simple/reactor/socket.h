#ifndef LIB_REACTOR_SOCKET_H
#define LIB_REACTOR_SOCKET_H

#include "buffer/buffer.h"
#include "event.h"

namespace simple::reactor {

    class socket {
        friend class connection;

        public:
            socket() {}
            virtual ~socket() {}

        public:
            virtual void open() {}
            virtual void close() {}

        public:
            virtual void do_write() = 0;

        protected:
            buffer::buffer<> recv_buff_;
            buffer::buffer<> send_buff_;

        protected:
            event::io *io_;
    };

}

#endif
