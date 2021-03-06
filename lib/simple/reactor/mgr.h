#ifndef LIB_REACTOR_MGR_H
#define LIB_REACTOR_MGR_H

#include "event.h"
#include "connection.h"

#include <unordered_set>
#include <memory>
#include <netinet/ip.h>

namespace simple::reactor {

    class mgr: public std::enable_shared_from_this<mgr> {

        public:
            template<class connection_type = connection>
            class connector: public connection_type {
                public:
                    connector(socket *socket): connection_type(socket) {
                    }
                public:
                    virtual void connect(const char *host, uint16 port) = 0;
            };

        public:
            class acceptor: public virtual event::base {
                public:
                    acceptor(simple::reactor::mgr *m): m_(m) {
                    }

                public:
                    virtual void listen(const char *host, uint16 port) = 0;

                protected:
                    virtual void on_connected(socket *) = 0;

                protected:
                    mgr* m_;
            };

        public:
            template <class acceptor_type> std::shared_ptr<acceptor_type> listen(const char *host, uint16 port) {
                auto acceptor = std::shared_ptr<acceptor_type>(new acceptor_type(this));
                acceptor->listen(host, port);
                return acceptor;
            }

            template <class connector_type> std::shared_ptr<connector_type> connect(const char *host, uint16 port) {
                auto connector = std::shared_ptr<connector_type>(new connector_type(this));
                connector->connect(host, port);
                return connector;
            }

        public:
            virtual void run() = 0;
            virtual void stop() = 0;

    };
};


#endif
