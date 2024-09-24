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
    zmq::socket_t entity_requester;  // REQ socket for sending entity updates
    int client_id;
    Entity &entity;                      // The entity this client controls

public:
    // Constructor
    Client(int id, Entity &entity_ref);

    // Perform handshake with the server
    void performHandshake();

    // Send entity's position data to server
    void sendEntityUpdate();

    // Receive entity updates from the server
    void receiveEntityUpdates(std::vector<Entity>& entities);
};