
#ifndef LIB_ASIO_REACTOR_KCP_CONNECTOR_H
#define LIB_ASIO_REACTOR_KCP_CONNECTOR_H


#include "socket.h"

#include "../udp/connector.h"

#include "../mgr.h"

#include <ikcp.h>

namespace asio::reactor::kcp {

class connector: public udp::connector {
    public:
        connector(simple::reactor::mgr* m): connector(static_cast<mgr*>(m)->io_service_)  {
        }
        connector(asio::io_service& io_service): udp::connector(new socket(io_service)) {
        }

    public:
        void update(uint32 ms) {
            static_cast<socket*>(get_socket())->update(ms);
        }

};

}

#endif
