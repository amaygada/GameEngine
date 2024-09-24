#include "server.hpp"

// Constructor definition
Server::Server(std::vector<Entity>& entities_ref) : 
    context(1), 
    handshake_responder(context, ZMQ_REP), 
    entity_publisher(context, ZMQ_PUB),
    entity_responder(context, ZMQ_REP),  // REP socket for entity updates
    entities(entities_ref)
{
    // Bind for handshake (REQ-REP)
    handshake_responder.bind("tcp://*:5555");

    // Bind for receiving entity updates (REQ-REP)
    entity_responder.bind("tcp://*:5556"); 

    // Bind for broadcasting entity data (PUB-SUB)
    entity_publisher.bind("tcp://*:5557");
}

// Perform the handshake with the client
void Server::performHandshakeServer() {
    zmq::message_t request;
    while (handshake_responder.recv(request, zmq::recv_flags::none)) {
        std::string client_msg(static_cast<char*>(request.data()), request.size());
        std::cout << "Handshake received from: " << client_msg << std::endl;

        // Respond with an acknowledgment
        std::string reply = "Handshake OK";
        zmq::message_t reply_msg(reply.size());
        memcpy(reply_msg.data(), reply.data(), reply.size());
        handshake_responder.send(reply_msg, zmq::send_flags::none);
    }
}

// Receive updates from clients and update the entities vector
void Server::receiveEntityUpdates() {
    zmq::message_t request;
    if (entity_responder.recv(request, zmq::recv_flags::none)) {
        std::string entity_data(static_cast<char*>(request.data()), request.size());
        std::cout << "Received entity update: " << entity_data << std::endl;

        // Parse the received data and update the entities vector
        int id, x, y;
        sscanf(entity_data.c_str(), "%d,%d,%d", &id, &x, &y);

        if (id >= 0 && static_cast<std::size_t>(id) < entities.size()) {
            entities[id].x = x;
            entities[id].y = y;
        } else {
            std::cerr << "Invalid entity ID received: " << id << std::endl;
        }

        // Respond with acknowledgment
        std::string reply = "Update received";
        zmq::message_t reply_msg(reply.size());
        memcpy(reply_msg.data(), reply.data(), reply.size());
        entity_responder.send(reply_msg, zmq::send_flags::none);
    }
}

// Broadcast updated entity positions to all clients
void Server::broadcastEntityUpdates() {
    // Prepare updated entity data to broadcast
    std::string broadcast_message;
    for (size_t i = 0; i < entities.size(); ++i) {
        broadcast_message += std::to_string(i) + "," + 
                             std::to_string(entities[i].x) + "," + 
                             std::to_string(entities[i].y) + ";";
    }

    zmq::message_t message(broadcast_message.size());
    memcpy(message.data(), broadcast_message.data(), broadcast_message.size());
    entity_publisher.send(message, zmq::send_flags::none);
}