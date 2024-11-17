#pragma once
#include <zmq.hpp>
#include <iostream>
#include <vector>
#include "message.hpp"
#include "./../../utils/entity.hpp"

class Client {
private:
    zmq::context_t context;
    zmq::socket_t req_rep;
    zmq::socket_t pub_sub;
    unordered_map<int, std::vector<Entity *>> entityMap;

public:
    int id;
    Entity *entity;
    Serializer serializer;
    MessageHandler messageHandler;
    zmq::socket_t push_pull;
    
    Client();
    void performHandshake(vector<Entity*>& E);
    void sendEntityUpdate();
    void receiveEntityUpdates();
    void uponTermination();
    unordered_map<int, std::vector<Entity *>>& getEntityMap();
    void setEntityMap(unordered_map<int, std::vector<Entity *>> entityMap);

};