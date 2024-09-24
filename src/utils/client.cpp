#include "client.hpp"

// Constructor definition
Client::Client(int id, Entity &entity_ref) : 
    context(1),
    handshake_requester(context, ZMQ_REQ),
    entity_subscriber(context, ZMQ_SUB),
    entity_requester(context, ZMQ_REQ),  // REQ socket for sending entity updates
    client_id(id),
    entity(entity_ref)
{
    // Connect for handshake (REQ-REP)
    handshake_requester.connect("tcp://localhost:5555");

    // Connect to server for sending entity updates (REQ-REP)
    entity_requester.connect("tcp://localhost:5556");

    // Connect for receiving entity updates (PUB-SUB)
    entity_subscriber.connect("tcp://localhost:5557");
    entity_subscriber.set(zmq::sockopt::subscribe, "");  // subscribing to all topics
}

// Method to perform handshake with the server
void Client::performHandshake() {
    std::string request_message = "Client " + std::to_string(client_id) + " handshake";
    zmq::message_t request(request_message.size());
    memcpy(request.data(), request_message.data(), request_message.size());
    handshake_requester.send(request, zmq::send_flags::none);

    zmq::message_t reply;
    zmq::recv_result_t result = handshake_requester.recv(reply, zmq::recv_flags::none);
    
    // Check if the receive operation was successful
    if (result) {
        std::string reply_str(static_cast<char*>(reply.data()), reply.size());
        std::cout << "Handshake reply: " << reply_str << std::endl;
    } else {
        std::cerr << "Failed to receive handshake reply from server" << std::endl;
    }
}

// Method to send entity's position data to the server
void Client::sendEntityUpdate() {
    // Send entity's position data to server
    std::string entity_data = std::to_string(client_id) + "," +
                              std::to_string(entity.x) + "," +
                              std::to_string(entity.y);
    zmq::message_t message(entity_data.size());
    memcpy(message.data(), entity_data.data(), entity_data.size());

    // Send request (entity update) to the server
    entity_requester.send(message, zmq::send_flags::none);

    // Wait for server acknowledgment
    zmq::message_t reply;
    zmq::recv_result_t result = entity_requester.recv(reply, zmq::recv_flags::none);
    if (result) {
        std::string reply_str(static_cast<char*>(reply.data()), reply.size());
        //std::cout << "Server reply: " << reply_str << std::endl;
    } else {
        //std::cerr << "Failed to receive reply from server" << std::endl;
    }
}

// Method to receive entity updates from the server
void Client::receiveEntityUpdates(std::vector<Entity>& entities) {
    zmq::message_t update;
    while (entity_subscriber.recv(update, zmq::recv_flags::dontwait)) {
        std::string entity_data(static_cast<char*>(update.data()), update.size());
        // std::cout << "Received entity update: " << entity_data << std::endl;

        // Parse the received data and update the entities vector
        size_t pos = 0;
        while ((pos = entity_data.find(";")) != std::string::npos) {
            std::string token = entity_data.substr(0, pos);
            int id, x, y;
            if (sscanf(token.c_str(), "%d,%d,%d", &id, &x, &y) == 3) {
                entities[id].x = x;
                entities[id].y = y;
            }
            entity_data.erase(0, pos + 1);
        }
    }
}