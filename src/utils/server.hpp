#pragma once
#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "entity.hpp"
#include <thread>
#include <mutex>
class Server {
private:
    zmq::context_t context;
    zmq::socket_t handshake_responder;  // For REQ-REP handshake
    zmq::socket_t entity_publisher;     // For broadcasting updated positions to clients
    zmq::socket_t entity_responder;     // For responding to entities after receiving the data
    std::vector<Entity> &entities;      // Reference to the main game loop's entities
    std::mutex entity_mutex;            // Mutex for synchronizing access to entities

public:
    // Constructor
    Server(std::vector<Entity>& entities_ref);

    // Main server run loop (handles clients and broadcasting)
    void run();

    // Function to handle a single client (runs in its own thread)
    void handleClient(int client_id);

    // Receive updates from clients and update the entities vector
    void receiveEntityUpdates();

    // Broadcast updated entity positions to all clients
    void broadcastEntityUpdates();
};