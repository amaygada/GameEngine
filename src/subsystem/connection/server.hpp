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
#include "./../event_manager/event_base.hpp"
#include "message.hpp"

class Server {
private:
    zmq::context_t context;
    zmq::socket_t pub_sub;
    zmq::socket_t push_pull;
    int id_counter = 0;
    int64_t broadcast_start_time = -1;

public:
    std::unordered_map<int, std::vector<Entity *>> entityMap;
    Serializer serializer;
    MessageHandler messageHandler;
    std::mutex mutex;
    Timeline *broadcastTimeline;
    zmq::socket_t req_rep;

    Server();
    void run();
    void handleRequest(string message);
    virtual void handleCustomRequest(string message){};
    void handleReqRep();
    void handlePushPull();
    void handlePubSub();
    void broadcastEntityUpdates();
    void addEntities(std::vector<Entity*> E);
    std::unordered_map<int, std::vector<Entity *>> getEntityMap();
    void setEntityMap(std::unordered_map<int, std::vector<Entity *>> entityMap);
};

extern Timeline *globalTimeline;
extern EventManager *eventManager;