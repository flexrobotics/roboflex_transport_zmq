#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "roboflex_core/core.h"
#include "roboflex_core/pybindings.h"
#include "roboflex_transport_zmq/zmq_nodes.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::transportzmq;

PYBIND11_MODULE(roboflex_transport_zmq_ext, m) {
    m.doc() = "roboflex_transport_zmq_ext";

    py::class_<zmq::context_t, std::shared_ptr<zmq::context_t>>(m, "ZMQContext")
        .def(py::init(&MakeZMQContext),
            "Create a zeromq context. Need for any subsequent zmq things.",
            py::arg("num_io_threads") = 1)
    ;

    py::class_<ZMQPublisher, core::Node, std::shared_ptr<ZMQPublisher>>(m, "ZMQPublisher")
        .def(py::init<ZMQContext,
                      const BindList&,
                      const std::string &,
                      unsigned int>(),
            "Creates a ZMQPublisher from a list of bind addresses.",
            py::arg("zmq_context"),
            py::arg("bind_addresses"),
            py::arg("name") = "ZMQPublisher",
            py::arg("max_queued_msgs") = 1000)
        .def(py::init<ZMQContext,
                      const std::string&,
                      const std::string&,
                      unsigned int>(),
            "Creates a ZMQPublisher from a single bind address.",
            py::arg("zmq_context"),
            py::arg("bind_address"),
            py::arg("name") = "ZMQPublisher",
            py::arg("max_queued_msgs") = 1000)
        .def_property_readonly("bind_addresses", &ZMQPublisher::get_bind_addresses)
        .def_property_readonly("max_queued_msgs", &ZMQPublisher::get_max_queued_msgs)
        .def("publish", &ZMQPublisher::publish)
        .def("publish", [](std::shared_ptr<ZMQPublisher> a, py::object m) {
            a->publish(dynoflex_from_object(m));
        })
    ;

    py::class_<ZMQSubscriber, core::RunnableNode, std::shared_ptr<ZMQSubscriber>>(m, "ZMQSubscriber")
        .def(py::init<ZMQContext,
                      const BindList&,
                      const std::string&,
                      unsigned int,
                      unsigned int>(),
            "Creates a ZMQSubscriber to a list of connect addresses.",
            py::arg("zmq_context"),
            py::arg("connect_addresses"),
            py::arg("name") = "ZMQSubscriber",
            py::arg("max_queued_msgs") = 1000,
            py::arg("timeout_milliseconds") = 10)

        .def(py::init<ZMQContext,
                      const std::string&,
                      const std::string&,
                      unsigned int,
                      unsigned int>(),
            "Creates a ZMQSubscriber to a single connect address.",
            py::arg("zmq_context"),
            py::arg("connect_address"),
            py::arg("name") = "ZMQSubscriber",
            py::arg("max_queued_msgs") = 1000,
            py::arg("timeout_milliseconds") = 10)

        .def("pull", &ZMQSubscriber::pull)
        .def("produce", &ZMQSubscriber::produce)

        .def_property_readonly("connect_addresses", &ZMQSubscriber::get_connect_addresses)
        .def_property_readonly("connect_address", &ZMQSubscriber::get_connect_address)
        .def_property_readonly("max_queued_msgs", &ZMQSubscriber::get_max_queued_msgs)
        .def_property_readonly("timeout_milliseconds", &ZMQSubscriber::get_timeout_milliseconds)
    ;

    py::class_<ZMQRequestClient, core::Node, std::shared_ptr<ZMQRequestClient>>(m, "ZMQRequestClient")
        .def(py::init<ZMQContext,
                      const BindList&,
                      const std::string&,
                      unsigned int>(),
            "Creates a ZMQ request client from a list of connect addresses.",
            py::arg("zmq_context"),
            py::arg("connect_addresses"),
            py::arg("name") = "ZMQRequestClient",
            py::arg("timeout_milliseconds") = 1000)
        .def(py::init<ZMQContext,
                      const std::string&,
                      const std::string&,
                      unsigned int>(),
            "Creates a ZMQ request client from a single connect address.",
            py::arg("zmq_context"),
            py::arg("connect_address"),
            py::arg("name") = "ZMQRequestClient",
            py::arg("timeout_milliseconds") = 1000)
        .def("call", &ZMQRequestClient::call,
            py::arg("message"),
            py::arg("timeout_milliseconds") = -1)
        .def("call", [](std::shared_ptr<ZMQRequestClient> a, py::object m, int timeout_milliseconds) {
            return a->call(dynoflex_from_object(m), timeout_milliseconds);
        },
            py::arg("message"),
            py::arg("timeout_milliseconds") = -1)
        .def_property_readonly("connect_addresses", &ZMQRequestClient::get_connect_addresses)
        .def_property_readonly("connect_address", &ZMQRequestClient::get_connect_address)
        .def_property_readonly("timeout_milliseconds", &ZMQRequestClient::get_timeout_milliseconds)
    ;

    py::class_<ZMQRequestServer, core::RunnableNode, std::shared_ptr<ZMQRequestServer>>(m, "ZMQRequestServer")
        .def(py::init<ZMQContext,
                      const BindList&,
                      const std::string&,
                      unsigned int,
                      ZMQRequestServer::RequestHandler>(),
            "Creates a ZMQ request server from a list of bind addresses.",
            py::arg("zmq_context"),
            py::arg("bind_addresses"),
            py::arg("name") = "ZMQRequestServer",
            py::arg("timeout_milliseconds") = 10,
            py::arg("request_handler") = nullptr)
        .def(py::init<ZMQContext,
                      const std::string&,
                      const std::string&,
                      unsigned int,
                      ZMQRequestServer::RequestHandler>(),
            "Creates a ZMQ request server from a single bind address.",
            py::arg("zmq_context"),
            py::arg("bind_address"),
            py::arg("name") = "ZMQRequestServer",
            py::arg("timeout_milliseconds") = 10,
            py::arg("request_handler") = nullptr)
        .def("set_handler", &ZMQRequestServer::set_handler)
        .def_property_readonly("bind_addresses", &ZMQRequestServer::get_bind_addresses)
        .def_property_readonly("timeout_milliseconds", &ZMQRequestServer::get_timeout_milliseconds)
    ;
}
