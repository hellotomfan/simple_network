#ifndef LIB_ASIO_REACTOR_KCP_CONNECTION_H
#define LIB_ASIO_REACTOR_KCP_CONNECTION_H


#include "../../simple/reactor/connection.h"


namespace asio::reactor::kcp {

class connection: public simple::reactor::connection {
    public:
        connection(socket *socket): simple::reactor::connection(socket) {
        }

    public:
        void update() {

        }

    private:
        void on_read() {
            //TODO
        }


};



}
















#endif
