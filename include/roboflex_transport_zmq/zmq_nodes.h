#ifndef ROBOFLEX_TRANSPORT_ZMQ_NODES__H
#define ROBOFLEX_TRANSPORT_ZMQ_NODES__H

#include <functional>
#include <memory>
#include <mutex>
#include <zmq.hpp>
#include "roboflex_core/core.h"

namespace roboflex {
namespace transportzmq {

using std::cout, std::cerr, std::string, std::list, core::Node, core::RunnableNode, core::MessagePtr;

using ZMQContext = shared_ptr<zmq::context_t>;

inline ZMQContext MakeZMQContext(unsigned int num_io_threads = 1) {
    cerr << "Made a new ZMQ Context with zmq version "
         << ZMQ_VERSION_MAJOR << "."
         << ZMQ_VERSION_MINOR << "."
         << ZMQ_VERSION_PATCH << endl;
    return make_shared<zmq::context_t>(num_io_threads);
}

/// A list of NICs i.e: { "inproc://sometqueuename", "ipc://somesocketname", "tcp://*:5432", etc }
typedef list<string> BindList;

/**
 * A node that publishes messages to a ZMQ socket. 
 */
class ZMQPublisher: public Node {
public:
    ZMQPublisher(
        ZMQContext context,
        const BindList& bind_addresses,
        const string& name = "ZMQPublisher",
        unsigned int max_queued_msgs = 1000);

    // A convenience constructor to bind to a single address.
    ZMQPublisher(
        ZMQContext context,
        const string& bind_address,
        const string& name = "ZMQPublisher",
        unsigned int max_queued_msgs = 1000):
            ZMQPublisher(context, BindList{bind_address}, name, max_queued_msgs) {}

    void receive(MessagePtr m) override;
    void publish(MessagePtr m) { this->signal_self(m); }

    const BindList & get_bind_addresses() const { return bind_addresses; }
    unsigned int get_max_queued_msgs() const { return max_queued_msgs; }

protected:

    void ensure_zmq_socket();

    ZMQContext context;
    BindList bind_addresses;
    unique_ptr<zmq::socket_t> socket;
    unsigned int max_queued_msgs;
};

using ZMQPublisherPtr = shared_ptr<ZMQPublisher>;

/**
 * A node that subscribes to a ZMQ socket and signals messages
 * it receives from it. Must be start()-ed.
 */
class ZMQSubscriber: public RunnableNode {
public:
    ZMQSubscriber(
        ZMQContext context,
        const BindList& connect_addresses,
        const string& name = "ZMQSubscriber",
        unsigned int max_queued_msgs = 1000,
        unsigned int timeout_milliseconds = 10);

    // a convenience constructor for a single connection address
    ZMQSubscriber(
        ZMQContext context,
        const string& connect_address,
        const string& name = "ZMQSubscriber",
        unsigned int max_queued_msgs = 1000,
        unsigned int timeout_milliseconds = 10):
            ZMQSubscriber(context, BindList{connect_address}, name, max_queued_msgs, timeout_milliseconds) {}

    virtual ~ZMQSubscriber();

    core::MessagePtr pull(int timeout_milliseconds=10);
    void produce(int timeout_millisecond=10);

    string get_connect_address() const { return connect_addresses.front(); }
    BindList get_connect_addresses() const { return connect_addresses; }
    unsigned int get_timeout_milliseconds() const { return _timeout_milliseconds; }
    unsigned int get_max_queued_msgs() const { return max_queued_msgs; }

protected:
    void ensure_sockets();
    void destroy_sockets();

    void child_thread_fn() override;

    ZMQContext context;
    BindList connect_addresses;
    vector<shared_ptr<zmq::socket_t>> sockets;
    unsigned int max_queued_msgs;
    unsigned int _timeout_milliseconds;
    vector<zmq::pollitem_t> pollable_socket_items;
    bool sockets_constructed;
};

using ZMQSubscriberPtr = shared_ptr<ZMQSubscriber>;

/**
 * A synchronous request client for roboflex messages over ZMQ REQ/REP.
 *
 * This is intentionally RPC-shaped, unlike ZMQPublisher/ZMQSubscriber.
 * call() sends one roboflex message and blocks until the matching reply
 * arrives or the timeout expires. The client serializes concurrent calls
 * with a mutex because a ZMQ_REQ socket supports one outstanding request.
 */
class ZMQRequestClient: public Node {
public:
    ZMQRequestClient(
        ZMQContext context,
        const BindList& connect_addresses,
        const string& name = "ZMQRequestClient",
        unsigned int timeout_milliseconds = 1000);

    // a convenience constructor for a single connection address
    ZMQRequestClient(
        ZMQContext context,
        const string& connect_address,
        const string& name = "ZMQRequestClient",
        unsigned int timeout_milliseconds = 1000):
            ZMQRequestClient(context, BindList{connect_address}, name, timeout_milliseconds) {}

    core::MessagePtr call(core::MessagePtr m, int timeout_milliseconds = -1);
    void receive(core::MessagePtr m) override;

    string get_connect_address() const { return connect_addresses.front(); }
    BindList get_connect_addresses() const { return connect_addresses; }
    unsigned int get_timeout_milliseconds() const { return default_timeout_milliseconds; }

protected:
    void ensure_socket();
    void reset_socket();

    ZMQContext context;
    BindList connect_addresses;
    unique_ptr<zmq::socket_t> socket;
    unsigned int default_timeout_milliseconds;
    std::mutex call_mutex;
};

using ZMQRequestClientPtr = shared_ptr<ZMQRequestClient>;

/**
 * A synchronous request server for roboflex messages over ZMQ REP.
 *
 * The server binds one or more addresses, receives request messages, calls
 * a handler, and sends the returned message as the reply. If no handler is
 * installed, handle_rpc(request) is used so subclasses can provide normal
 * roboflex RPC behavior.
 */
class ZMQRequestServer: public RunnableNode {
public:
    using RequestHandler = std::function<core::MessagePtr(core::MessagePtr)>;

    ZMQRequestServer(
        ZMQContext context,
        const BindList& bind_addresses,
        const string& name = "ZMQRequestServer",
        unsigned int timeout_milliseconds = 10,
        RequestHandler request_handler = nullptr);

    // a convenience constructor for a single bind address
    ZMQRequestServer(
        ZMQContext context,
        const string& bind_address,
        const string& name = "ZMQRequestServer",
        unsigned int timeout_milliseconds = 10,
        RequestHandler request_handler = nullptr):
            ZMQRequestServer(context, BindList{bind_address}, name, timeout_milliseconds, request_handler) {}

    void set_handler(RequestHandler request_handler) { this->request_handler = request_handler; }

    const BindList & get_bind_addresses() const { return bind_addresses; }
    unsigned int get_timeout_milliseconds() const { return timeout_milliseconds; }

protected:
    void ensure_socket();
    void destroy_socket();
    void child_thread_fn() override;

    ZMQContext context;
    BindList bind_addresses;
    unique_ptr<zmq::socket_t> socket;
    unsigned int timeout_milliseconds;
    RequestHandler request_handler;
};

using ZMQRequestServerPtr = shared_ptr<ZMQRequestServer>;


} // namespace transportzmq
} // namespace roboflex

#endif // ROBOFLEX_TRANSPORT_ZMQ_NODES__H
