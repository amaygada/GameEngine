#include "message.hpp"

void MessageHandler::sendMessage(zmq::socket_t &sock, string message) {
    zmq::message_t request(message.size());
    memcpy(request.data(), message.c_str(), message.size());
    sock.send(request, zmq::send_flags::none);
}

string MessageHandler::receiveMessageLoop(zmq::socket_t &sock, zmq::recv_flags flags) {
    zmq::message_t reply;
    string message;
    while(sock.recv(reply, flags)){
        message = string(static_cast<char*>(reply.data()), reply.size());
    }
    return message;
}

string MessageHandler::receiveMessage(zmq::socket_t &sock, zmq::recv_flags flags) {
    zmq::message_t reply;
    if(sock.recv(reply, flags)){
        string message = string(static_cast<char*>(reply.data()), reply.size());
        return message;
    }
}

string MessageHandler::receiveMessage(zmq::socket_t &sock) {
    zmq::message_t reply;
    if(sock.recv(reply, zmq::recv_flags::none)){
        string message = string(static_cast<char*>(reply.data()), reply.size());
        return message;
    }
}

string MessageHandler::createMessage(int type, string data) {
    return "Type:" + std::to_string(type) + " Data:" + data;
}

pair<int, string> MessageHandler::parseMessage(string message) {
    int type;
    sscanf(message.c_str(), "Type:%d", &type);

    size_t dataPos = message.find("Data:");
    std::string data;
    if (dataPos != std::string::npos) {data = message.substr(dataPos + 5); }
    
    return std::make_pair(type, data);
}

std::string Serializer::serializeEntity(Entity *entity) {
    string request_message = "Entity data: x=" + std::to_string(entity->x) +
                                ", y=" + std::to_string(entity->y) +
                                ", w=" + std::to_string(entity->w) +
                                ", h=" + std::to_string(entity->h) +
                                ", r=" + std::to_string(entity->color.r) +
                                ", g=" + std::to_string(entity->color.g) +
                                ", b=" + std::to_string(entity->color.b) +
                                ", a=" + std::to_string(entity->color.a);
    return request_message;
}

Entity* Serializer::deserializeEntity(std::string data) {
    int x, y, w, h, r, g, b, a;
    sscanf(data.c_str(), "Entity data: x=%d, y=%d, w=%d, h=%d, r=%d, g=%d, b=%d, a=%d", &x, &y, &w, &h, &r, &g, &b, &a);
    SDL_Color color = {r, g, b, a};
    Entity *entity = new Entity(x, y, w, h, color);
    return entity;
}