#pragma once
#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <map>

#include "./../../utils/entity.hpp"
#include "message.hpp"

class Server {
private:
    zmq::context_t context;
    zmq::socket_t req_rep;
    zmq::socket_t pub_sub;
    zmq::socket_t push_pull;
    std::mutex mutex;
    std::unordered_map<int, std::vector<Entity *>> entityMap;
    int id_counter = 0;
    Serializer serializer;
    MessageHandler messageHandler;

    Timeline *broadcastTimeline;
    int64_t broadcast_start_time = -1;

public:
    Server();
    void run();
    void handleRequest(string message);
    void handleReqRep();
    void handlePushPull();
    void handlePubSub();
    void broadcastEntityUpdates();
    void addEntities(std::vector<Entity*> E);
    std::unordered_map<int, std::vector<Entity *>> getEntityMap();
};

extern Timeline *globalTimeline;
extern Timeline *gameTimeline;

extern InputSubsystem *inputSubsystem;
extern PhysicsSubsystem *physicsSubsystem;
extern CollisionSubsystem *collisionSubsystem;
extern AnimationSubsystem *animationSubsystem;