#pragma once
#include <zmq.hpp>
#include <vector>
#include <string>

class P2PClient {
    zmq::context_t context;
    zmq::socket_t my_socket;
    std::vector<zmq::socket_t> network_sockets;

public:
    P2PClient();
    void listen();
    void connect();
    void negotiate_positions();
    void handle_request(std::string message);
    void send_entity_update();

};