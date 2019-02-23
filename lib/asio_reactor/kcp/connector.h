
#ifndef LIB_ASIO_REACTOR_KCP_CONNECTOR_H
#define LIB_ASIO_REACTOR_KCP_CONNECTOR_H


#include "socket.h"

#include "../udp/connector.h"

#include "../mgr.h"

#include <ikcp.h>

namespace asio::reactor::kcp {

class connector: public udp::connector, public simple::reactor::event::timer {

    public:
        connector(simple::reactor::mgr* m): connector(static_cast<mgr*>(m)->io_service_) {
            mgr_ = static_cast<mgr*>(m);
            mgr_->add(this);
        }
        connector(asio::io_service& io_service): udp::connector(new socket(io_service)) {
        }
        ~connector() {
            mgr_->remove(this);
        }

    protected:
        void on_time(uint32 current) {
            static_cast<socket*>(get_socket())->update(current);
        }

    private:
        mgr *mgr_;

};

}

#endif
