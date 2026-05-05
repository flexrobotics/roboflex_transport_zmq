#include <iostream>
#include <memory>
#include <string>

#include "roboflex_core/core_nodes/callback_fun.h"
#include "roboflex_core/core_messages/core_messages.h"
#include "roboflex_core/util/utils.h"
#include "roboflex_transport_zmq/zmq_nodes.h"

#define REQUIRE(condition) do { \
    if (!(condition)) { \
        std::cerr << "FAILED: " #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return 1; \
    } \
} while (0)

using namespace roboflex;

namespace {

core::MessagePtr make_string_message(const std::string& value)
{
    return std::make_shared<core::StringMessage>("test_string", value);
}

int test_manual_pull_without_observers()
{
    auto context = transportzmq::MakeZMQContext();
    const std::string endpoint = "inproc://test_manual_pull_without_observers";

    transportzmq::ZMQPublisher publisher(context, endpoint);
    transportzmq::ZMQSubscriber subscriber(context, endpoint);

    // Construct and connect the subscriber socket before publishing.
    (void) subscriber.pull(1);
    core::sleep_ms(50);

    core::MessagePtr received = nullptr;
    for (int i = 0; i < 50 && received == nullptr; ++i) {
        publisher.publish(make_string_message("manual-pull"));
        received = subscriber.pull(20);
        core::sleep_ms(5);
    }

    REQUIRE(received != nullptr);
    core::StringMessage decoded(*received);
    REQUIRE(decoded.message() == "manual-pull");
    return 0;
}

int test_threaded_pub_sub_signal()
{
    auto context = transportzmq::MakeZMQContext();
    const std::string endpoint = "inproc://test_threaded_pub_sub_signal";

    transportzmq::ZMQPublisher publisher(context, endpoint);
    transportzmq::ZMQSubscriber subscriber(context, endpoint, "TestZMQSubscriber", 100, 5);

    bool got_message = false;
    nodes::CallbackFun callback([&](core::MessagePtr m) {
        if (m != nullptr) {
            core::StringMessage decoded(*m);
            got_message = decoded.message() == "threaded-pub-sub";
        }
    });

    subscriber > callback;
    subscriber.start();
    core::sleep_ms(50);

    for (int i = 0; i < 50 && !got_message; ++i) {
        publisher.publish(make_string_message("threaded-pub-sub"));
        core::sleep_ms(10);
    }

    subscriber.stop();
    REQUIRE(got_message);
    return 0;
}

int test_request_server_round_trip()
{
    auto context = transportzmq::MakeZMQContext();
    const std::string endpoint = "inproc://test_request_server_round_trip";

    transportzmq::ZMQRequestServer server(
        context,
        endpoint,
        "TestZMQRequestServer",
        10,
        [](core::MessagePtr request) {
            core::StringMessage decoded(*request);
            return make_string_message("reply:" + decoded.message());
        });
    transportzmq::ZMQRequestClient client(context, endpoint, "TestZMQRequestClient", 1000);

    server.start();
    core::sleep_ms(50);

    auto response = client.call(make_string_message("request"), 1000);
    server.stop();

    REQUIRE(response != nullptr);
    core::StringMessage decoded(*response);
    REQUIRE(decoded.message() == "reply:request");
    return 0;
}

int test_request_timeout()
{
    auto context = transportzmq::MakeZMQContext();
    transportzmq::ZMQRequestClient client(
        context,
        "inproc://test_request_timeout_without_server",
        "TestZMQRequestTimeoutClient",
        50);

    auto response = client.call(make_string_message("request"), 50);
    REQUIRE(response == nullptr);
    return 0;
}

} // namespace

int main()
{
    REQUIRE(test_manual_pull_without_observers() == 0);
    REQUIRE(test_threaded_pub_sub_signal() == 0);
    REQUIRE(test_request_server_round_trip() == 0);
    REQUIRE(test_request_timeout() == 0);
    return 0;
}
