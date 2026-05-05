# roboflex.transport.zmq

Roboflex support for the ZMQ transport.

    any node -> ZMQPublisher ==THEINTERNET==> ZMQSubscriber -> any node

See https://zeromq.org/ for details.

Using ZMQ, nodes can connect to other nodes, running in different threads, different processes, or different computers, with a publisher-subscriber pattern. roboflex.transport.zmq supports:

    "inproc" transport -> between threads within same process
    "ipc" transport -> between processes on same computer
    "tcp" transport -> between processes on different computers


## System Dependencies

    None! We build libzmq from source...

## pip install

    pip install roboflex.transport.zmq

## Import (python)

    import roboflex.transport.zmq as rtz

## Build  (for c++ projects):

    mkdir build && cd build
    cmake ..
    make
    make install

Python bindings are off by default for plain C++ CMake builds. To build them with CMake directly:

    cmake -S . -B build -DBUILD_ROBOFLEX_TRANSPORT_ZMQ_PYTHON_EXT=ON

## Run Tests

Tests are plain C++ executables registered with CTest; no separate testing library is required.

    cmake -S . -B build
    cmake --build build --target test_zmq_transport
    ctest --test-dir build --output-on-failure

## Run Examples (see [examples](examples))

    go to roboflex_transport_zmq/examples

    ... create and activate some sort of virtual environment
    where you installed roboflex.transport.zmq...

    python pub_sub_0_py.py

## Nodes:

There are five: `ZMQContext`, `ZMQPublisher`, `ZMQSubscriber`, `ZMQRequestClient`, and `ZMQRequestServer`.

To use the ZMQ transport nodes, first you must create a ZMQContext object. This mirrors the design of ZMQ itself.

    # all parameters optional
    zmq_context = ZMQContext(
        num_io_threads = 1,
    )

First, know this. "bind addresses" in this world can be three different things. All are strings, but can create different types of queues. These all implement one-to-many publish-subscribe pattern (in fact, it's actually many-to-many).

    1. thread-to-thread only queues; "inproc://somename"; the fastest.
    2. process-to-process (or thread-to-thread) queues; "ipc://somename"; sort of fast.
    3. computer-to-computer (can work anywhere) queues (uses TCP): "tcp://*:5647"; the slowest, but works across the planet.

Then, create a ZMQPublisher:

    zmq_pub = ZMQPublisher(
        # the ZMQContext object you created
        zmq_context, 

        # what socket to bind to, or what transport to publish on
        bind_address = <bind address>,
        #    or
        bind_addresses = [<bind address>],

        # optional
        
        # name of the
        name = "ZMQPublisher",

        # same as 'high-water mark' in zeromq parlance
        max_queued_msgs = 1000,
    )

    #... when a ZMQPublisher receives a message from some upstream node, #it will wire-serialize it, and publish on its transport.

    #You can get the bind_addresses:

    ba = zmq_pub.bind_addresses

    # you can get the high-water mark
    hm = zmq_pub.max_queued_msgs

    # You can publish a message 'by hand' - same as calling 'receive' on the node.
    zmq_pub.publish(some_message)

Then, create one or more ZMQSubscribers, to listen to what you are publishing. ZMQSubscribes are the equivalent of 'sensors' in that the are root nodes, must be started, and start a thread.

    zmq_sub = ZMQSubscriber(
        # the ZMQContext object you created
        zmq_context, 

        # what socket to bind to, or what transport to subscribe on
        connect_address = <bind address>,
        #    or
        connect_addresses = [<bind address>],

        # optional
        
        # name of the
        name = "ZMQPublisher",

        # same as 'high-water mark' in zeromq parlance
        max_queued_msgs = 1000,

        # how often to yield control on the thread
        # You'll probably never change this.
        timeout_milliseconds = 10,
    )

    # you get get values
    zmq_sub.connect_addresses
    zmq_sub.connect_address
    zmq_sub.max_queued_msgs
    zmq_sub.timeout_milliseconds

    # you MUST start it!
    zmq_sub.start()

    # you may pull a message 'by hand':
    msg_or_none = zmq_sub.pull(
        10, # timeout_milliseconds - how long to wait for a message
    )

    # you may 'produce' messages 'by hand' - this will wait x milliseconds
    # for one message, and if it has received one, signals it downstream
    zmq_sub.produce(
        10, # timeout_milliseconds
    )

## RPC / Request-Reply

For request-response workloads, use `ZMQRequestClient` and `ZMQRequestServer` instead of PUB/SUB.
This avoids subscriber lazy-join behavior and gives the caller a normal blocking call.

These classes use ZMQ REQ/REP. A client has one outstanding request at a time; concurrent calls are serialized.

    def handler(msg):
        # inspect the request and return a roboflex Message
        return msg

    ctx = ZMQContext()

    server = ZMQRequestServer(
        ctx,
        "tcp://*:5555",
        request_handler = handler,
    )
    server.start()

    client = ZMQRequestClient(
        ctx,
        "tcp://127.0.0.1:5555",
        timeout_milliseconds = 1000,
    )

    reply = client.call({"hello": "world"}, timeout_milliseconds = 1000)

`ZMQRequestClient` is also a node: when it receives a message, it performs a blocking call and signals the response downstream.
