#ifndef LIB_ASIO_REACTOR_KCP_CONNECTION_H
#define LIB_ASIO_REACTOR_KCP_CONNECTION_H

#include "socket.h"

#include "../../simple/reactor/socket.h"
#include "../../simple/reactor/connection.h"


namespace asio::reactor::kcp {

class connection: public simple::reactor::connection {
    public:
        connection(simple::reactor::socket *socket): simple::reactor::connection(socket) {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

};



}
















#endif
