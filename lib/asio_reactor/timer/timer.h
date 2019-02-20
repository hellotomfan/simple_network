#ifndef LIB_ASIO_REACTOR_TIMER_TIMER_H
#define LIB_ASIO_REACTOR_TIMER_TIMER_H

#include "base.h"
#include "../mgr.h"

#include "../../simple/reactor/event.h"

#include <asio.hpp>

#include <chrono>

namespace asio::reactor::timer {

class timer: public base {
    public:
        timer(simple::reactor::mgr *m): base(static_cast<mgr*>(m)->io_service_) {
        }
};

}

#endif
