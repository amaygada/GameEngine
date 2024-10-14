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
    zmq::socket_t push_pull;
    Entity *entity;
    unordered_map<int, std::vector<Entity *>> entityMap;
    Serializer serializer;
    MessageHandler messageHandler;

public:
    int id;

    Client();
    void performHandshake(vector<Entity*>& E);
    void sendEntityUpdate();
    void receiveEntityUpdates();
    void sendQuitMessage();
    unordered_map<int, std::vector<Entity *>>& getEntityMap();

};