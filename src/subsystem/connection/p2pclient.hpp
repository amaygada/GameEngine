#pragma once
#include <zmq.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <thread>
#include <mutex>

#include "./../../utils/entity.hpp"
#include "message.hpp"

class P2PClient {
    int id=-1;
    zmq::context_t context;
    zmq::socket_t my_master_socket;
    zmq::socket_t my_pub_socket;
    zmq::socket_t my_rep_socket;
    unordered_map<int, zmq::socket_t> sub_socket_map;
    unordered_map<int, zmq::socket_t> req_socket_map;
    Timeline *negotiateTimeline;
    int64_t negotiate_start_time = -1;
    MessageHandler messageHandler;
    Serializer serializer;
    int id_counter = 1;
    std::mutex mutex;

public:
    P2PClient();
    unordered_map<int, std::vector<Entity *>> entityMap;
    void expose_port_and_connect();
    void request_id();
    int get_id();
    void update_id(int id);
    void bind_publisher();
    void bind_rep_socket();
    void subscribe_to_peer(int peer_id);
    void connect_to_peer(int peer_id);
    void listen_on_detached_thread(zmq::socket_t &socket);
    void handle_req_rep(string message);
    string get_comma_sep_sockets();
    void announce_new_peer();
    void publish_entity_positions();
    void receive_entity_positions();
    void negotiate_platform_position(vector<Entity*> pe);

    unordered_map<int, std::vector<Entity *>> get_entity_map();
};

extern Timeline *globalTimeline;