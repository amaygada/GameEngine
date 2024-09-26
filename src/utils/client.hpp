#pragma once
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "entity.hpp"

class Client {
private:
    zmq::context_t context;
    zmq::socket_t handshake_requester;   // For REQ-REP handshake
    zmq::socket_t entity_subscriber;     // For receiving entity updates from server
    zmq::socket_t entity_publisher;  // REQ socket for sending entity updates
    unordered_map<int, std::vector<Entity *>> entityMap;
    Entity* entity;
    int client_id;

public:
    // Constructor
    Client(Entity* &entity_ref);

    // Perform handshake with the server
    void performHandshake();

    // Send entity's position data to server
    void sendEntityUpdate();

    // Receive entity updates from the server
    void receiveEntityUpdates();

    // Getter for entity map
    unordered_map<int, std::vector<Entity *>>& getEntityMap();
};