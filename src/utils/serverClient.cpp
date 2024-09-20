#include "serverClient.hpp"

ServerClient::ServerClient(char *type) {

    if (type == "server") {

        isClient = false;
        entities = {};
        zmq::context_t context(1);
        zmq::socket_t req_socket(context, zmq::socket_type::rep);
        req_socket.bind ("tcp://*:5555");
        zmq::socket_t pub_socket(context, zmq::socket_type::pub);
        pub_socket.bind("tcp://*:5555");

    }
    else if (type == "client") {

        isClient = true;
        entities = {};
        zmq::context_t context(1);
        zmq::socket_t req_socket(context, zmq::socket_type::rep);
        req_socket.bind ("tcp://*:5555");
        zmq::socket_t pub_socket(context, zmq::socket_type::pub);
        pub_socket.bind("tcp://*:5555");

    }
    else {
        
    }

}