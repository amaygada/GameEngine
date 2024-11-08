#pragma once
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <utility>
#include "./../../utils/entity.hpp"

class MessageHandler {
public:
    string createMessage(int type, string data);
    pair<int, string> parseMessage(string message);
    void sendMessage(zmq::socket_t &sock, string message);
    string receiveMessage(zmq::socket_t &sock);
    string receiveMessage(zmq::socket_t &sock, zmq::recv_flags flags);
    string receiveMessageLoop(zmq::socket_t &sock, zmq::recv_flags flags);
};

class Serializer {
public:
    std::string serializeEntity(Entity *entity);
    Entity* deserializeEntity(std::string data);
    std::string serializeEvent(Event *event, int time);
    Event* deserializeEvent(const std::string& serializedMessage);
};