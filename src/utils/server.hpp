#pragma once
#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "entity.hpp"
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <map>
#include "../subsystem/input_handling/input.hpp"
#include "../subsystem/physics/physics.hpp"
#include "../subsystem/rendering/render.hpp"
#include "../subsystem/collision/collision.hpp"

extern InputSubsystem *inputSubsystem;
extern PhysicsSubsystem *physicsSubsystem;
extern AnimationSubsystem *animationSubsystem;
extern CollisionSubsystem *collisionSubsystem;

class Server {
private:
    zmq::context_t context;
    zmq::socket_t handshake_responder;  // For REQ-REP handshake
    zmq::socket_t entity_publisher;     // For broadcasting updated positions to clients
    zmq::socket_t entity_responder;     // For  pulling entity data from the clients
    std::mutex entity_mutex;            // Mutex for synchronizing access to entities
    // unordered_map<int, Entity*> entityMap;
    unordered_map<int, std::vector<Entity *>> entityMap;

public:
    // Constructor
    Server();

    // Main server run loop (handles clients and broadcasting)
    void run();

    // Function to handle a single client (runs in its own thread)
    void handleClient(int client_id);

    // Receive updates from clients and update the entities vector
    void receiveEntityUpdates();

    // Broadcast updated entity positions to all clients
    void broadcastEntityUpdates();

    // Adds the given entities to the server's stored entities, as server-side entities
    void addEntities(std::vector<Entity*> E);

    unordered_map<int, std::vector<Entity *>> getEntityMap();
};