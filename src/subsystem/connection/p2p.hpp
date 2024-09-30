#pragma once
#include <zmq.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>

#include "./../../utils/entity.hpp"
#include "message.hpp"

class P2PClient {
    int id=-1;
    zmq::context_t context;
    zmq::socket_t my_req_rep_socket;
    zmq::socket_t my_pub_socket;
    zmq::socket_t peer_rep_socket;
    unordered_map<int, zmq::socket_t> sub_socket_map;
    unordered_map<int, zmq::socket_t> req_socket_map;

    unordered_map<int, std::vector<Entity *>> entityMap;
    MessageHandler messageHandler;
    Serializer serializer;
    int id_counter = 1;
    std::mutex mutex;

public:
    P2PClient();
    void update_id(int id);
    int get_id();
    void expose_port_and_connect();
    void send_id_request();
    unordered_map<int, std::vector<Entity *>> get_entity_map();
    unordered_map<int, zmq::socket_t>& get_socket_map();

    void listen_for_new_peers(zmq::socket_t &socket);
    void handle_master_req_rep(string message);
    void bind_publisher();
    string get_comma_sep_sockets();
    void handle_sub_requests(string message);

    void listen();
    void connect();
    void negotiate_positions();
    void handle_request(std::string message);
    void send_entity_update();


};