# include "client.hpp"

Client *Client::myself = nullptr;

Client::Client() : 
    context(1),
    req_rep(context, ZMQ_REQ),
    pub_sub(context, ZMQ_SUB),
    push_pull(context, ZMQ_PUSH)
    {
        // Connect to the server for handshake (REQ-REP)
        req_rep.connect("tcp://localhost:5555");
        push_pull.connect("tcp://localhost:5556");
        pub_sub.connect("tcp://localhost:5557");
        pub_sub.set(zmq::sockopt::subscribe, ""); // Subscribe to all messages

        myself = this;

        struct sigaction a;
        a.sa_handler = uponTermination;
        sigaction(SIGINT, &a, nullptr);

    }

void Client::performHandshake(vector<Entity*>& E){
    // get client Id
    string message = messageHandler.createMessage(1, "Hello");
    messageHandler.sendMessage(req_rep, message);
    string reply = messageHandler.receiveMessage(req_rep);
    auto msg = messageHandler.parseMessage(reply);
    auto idIndex = msg.second.find(" ");
    auto infoStr = msg.second.substr(0, idIndex);
    // id = std::stoi(msg.second);
    id = std::stoi(infoStr);
    entity = E[0];
    
    // send entity
    message = messageHandler.createMessage(2, "ClientID:"+msg.second+" Entity:"+serializer.serializeEntity(entity));
    messageHandler.sendMessage(req_rep, message);
    reply = messageHandler.receiveMessage(req_rep);
    // populate entity map
    entityMap[id] = {E[0]};
}

void Client::sendEntityUpdate(){
    string entity_str = serializer.serializeEntity(entity);
    string id_str = std::to_string(id);
    string message = messageHandler.createMessage(3, "ClientID:"+id_str+" Entity:"+entity_str);
    messageHandler.sendMessage(push_pull, message);
}

void Client::receiveEntityUpdates(){
    string broadcast_data;
    broadcast_data = messageHandler.receiveMessageLoop(pub_sub, zmq::recv_flags::dontwait);
    if(broadcast_data == "") return;
    std::istringstream iss(broadcast_data);
    std::string entity_update;
    vector<Entity*> temp = entityMap[id];
    entityMap.clear();
    entityMap[id] = temp;
    while (std::getline(iss, entity_update, ';')) {
        auto msg = messageHandler.parseMessage(entity_update);
        int client_id;
        char entity_data[256];
        sscanf(msg.second.c_str(), "ClientID:%d Entity:%255[^\n]", &client_id, &entity_data);
        Entity *entity = serializer.deserializeEntity(entity_data);
        if(client_id == id) continue;
        entityMap[client_id].push_back(entity);
    }
}

void Client::uponTermination(int signal) {

    if (myself && signal == SIGINT) {

        string message = myself->messageHandler.createMessage(0, "ClientID:" + std::to_string(myself->id));
        myself->messageHandler.sendMessage(myself->req_rep, message);

        myself->messageHandler.receiveMessage(myself->req_rep);

        exit(0);

    }

}

// Getter for entity_map
unordered_map<int, std::vector<Entity *>>& Client::getEntityMap() {
    return entityMap;
}