#include "p2pclient.hpp"

P2PClient::P2PClient() : 
    context(1),
    my_socket(context, ZMQ_REP)
    {
        my_socket.bind("tcp://*:5000");
    }

void P2PClient::listen(){
    zmq::message_t request;
    while(1){
        if(my_socket.recv(request, zmq::recv_flags::none)){
            std::string message = std::string(static_cast<char*>(request.data()), request.size());
            handle_request(message);
        }
    }
}