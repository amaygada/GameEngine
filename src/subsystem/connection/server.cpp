#include "server.hpp"
#include <iostream>

Server::Server() : 
    context(1),
    req_rep(context, ZMQ_REP),
    pub_sub(context, ZMQ_PUB),
    push_pull(context, ZMQ_PULL)
    {
        // Bind for handshake (REQ-REP)
        req_rep.bind("tcp://*:5555");
        push_pull.bind("tcp://*:5556");
        pub_sub.bind("tcp://*:5557");

        broadcastTimeline = new Timeline(globalTimeline, 1e9/100);

        gameTimeline->pause();
    }

void Server::broadcastEntityUpdates(){
    if (this->broadcast_start_time == -1) {
        this->broadcast_start_time = broadcastTimeline->getTime();
    }
    int64_t currentTime = broadcastTimeline->getTime();
    if (currentTime - this->broadcast_start_time < 1) return;
    this->broadcast_start_time = currentTime;

    string broadcast_message;
    for (const auto& pair : entityMap) {
        int client_id = pair.first;
        vector<Entity*> entities = pair.second;
        for (Entity* entity : entities) {
            broadcast_message += messageHandler.createMessage(4, "ClientID:" + std::to_string(client_id) + " Entity:" + serializer.serializeEntity(entity) + ";");
        }
    }    
    if(broadcast_message.empty()) return;
    messageHandler.sendMessage(pub_sub, broadcast_message);
}

void Server::handleReqRep(){
    zmq::message_t request;
    while(1){
        if(req_rep.recv(request, zmq::recv_flags::none)){
            std::string message = std::string(static_cast<char*>(request.data()), request.size());
            handleRequest(message); 
        }
    }
}

void Server::handlePushPull(){
    zmq::message_t request;
    while(1){
        if(push_pull.recv(request, zmq::recv_flags::none)){
            std::string message = std::string(static_cast<char*>(request.data()), request.size());
            handleRequest(message);
        }
    }
}

void Server::handlePubSub(){
    while(1){
        broadcastEntityUpdates();
    }
}

void Server::run(){

    std::thread req_rep_thread(&Server::handleReqRep, this);
    std::thread push_pull_thread(&Server::handlePushPull, this);
    std::thread pub_sub_thread(&Server::handlePubSub, this);

    req_rep_thread.detach();
    push_pull_thread.detach();
    pub_sub_thread.detach();
}


void Server::handleRequest(string message){
    auto msg = messageHandler.parseMessage(message);
    int type = msg.first;
    string data = msg.second;

     if (type == 0) {
        Event *e = serializer.deserializeEvent(data);
        eventManager->raiseEvent(e, 0);   
        // int client_id;
        // sscanf(data.c_str(), "ClientID:%d", &client_id);
        // auto it = entityMap.find(client_id);
        // entityMap.erase(it);
    }

    // HANDSHAKE REQUEST
    else if( type == 1 ){
        // get id in a thread safe manner
        mutex.lock();
        int id = id_counter++;
        mutex.unlock();
        // send id to client
        string id_str = std::to_string(id);
        string reply = messageHandler.createMessage(1, id_str);
        messageHandler.sendMessage(req_rep, reply);
    }

    // Entity introduction request
    else if( type == 2 ){
        int client_id;
        char entity_data[256];
        sscanf(data.c_str(), "ClientID:%d Entity:%255[^\n]", &client_id, &entity_data);
        Entity *entity = serializer.deserializeEntity(entity_data);
        std::vector<Entity*> E;
        E.push_back(entity);
        entityMap[client_id] = E;
        string reply = messageHandler.createMessage(2, "Entity received");
        messageHandler.sendMessage(req_rep, reply);
        if(gameTimeline->isPaused()) gameTimeline->resume();
    }

    // Entity update request
    else if( type == 3 ){
        int client_id;
        char entity_data[256];
        sscanf(data.c_str(), "ClientID:%d Entity:%255[^\n]", &client_id, &entity_data);
        Entity *entity = serializer.deserializeEntity(entity_data);
        mutex.lock();
        entityMap[client_id][0] = entity;
        mutex.unlock();
    }
}

void Server::addEntities(std::vector<Entity*> E){
    for (Entity* entity : E) {
        entityMap[-1].push_back(entity);
    }
}