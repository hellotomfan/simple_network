#ifndef LIB_REACTOR_EVENT_H
#define LIB_REACTOR_EVENT_H

#include "../../common.h"

#include <memory>

namespace simple::reactor {
    namespace event {

        class base: public std::enable_shared_from_this<base> {
            public:
                virtual ~base() {}
        };

        class io: public virtual base {
            public:
                virtual void on_read() = 0;
                virtual void on_write() = 0;
                virtual void on_close() = 0;
        };

        class timer: public virtual base {
            public:
                virtual void on_time(uint32 current) = 0;
        };
    };

};


#endif
