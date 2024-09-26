#include "client.hpp"

// Constructor definition
Client::Client(Entity* &entity_ref) : 
    context(1),
    handshake_requester(context, ZMQ_REQ),
    entity_subscriber(context, ZMQ_SUB),
    entity_publisher(context, ZMQ_PUSH),
    entity(entity_ref)
{
    // Connect for handshake (REQ-REP)
    handshake_requester.connect("tcp://localhost:5555");

    // Connect to server for sending entity updates (REQ-REP)
    entity_publisher.connect("tcp://localhost:5556");

    // Connect for receiving entity updates (PUB-SUB)
    entity_subscriber.connect("tcp://localhost:5557");
    entity_subscriber.set(zmq::sockopt::subscribe, "");  // subscribing to all topics
}

// Method to perform handshake with the server
void Client::performHandshake() {

    // std::cout << "Sending entity data: x=" << entity->x << ", y=" << entity->y << std::endl;
    // Send initial entity data (x, y, w, h, r, g, b, a) to the server
    std::string request_message = "Entity data: x=" + std::to_string(entity->x) +
                                ", y=" + std::to_string(entity->y) +
                                ", w=" + std::to_string(entity->w) +
                                ", h=" + std::to_string(entity->h) +
                                ", r=" + std::to_string(entity->color.r) +
                                ", g=" + std::to_string(entity->color.g) +
                                ", b=" + std::to_string(entity->color.b) +
                                ", a=" + std::to_string(entity->color.a);

    zmq::message_t request(request_message.size());
    memcpy(request.data(), request_message.data(), request_message.size());
    handshake_requester.send(request, zmq::send_flags::none);

    // Receive the assigned client ID from the server
    zmq::message_t reply;
    zmq::recv_result_t result = handshake_requester.recv(reply, zmq::recv_flags::none);

    // Check if the receive operation was successful
    if (result) {
        std::string reply_str(static_cast<char*>(reply.data()), reply.size());
        // std::cout << "Handshake reply: " << reply_str << std::endl;

        // Extract the assigned client ID from the server's reply
        std::size_t pos = reply_str.find("client ID: ");
        if (pos != std::string::npos) {
            std::string id_str = reply_str.substr(pos + 11);  // Extract client ID after "client ID: "
            // printf("Client ID substring: %s\n", id_str.c_str()); // TODO remove this
            client_id = std::stoi(id_str);  // Convert client ID from string to integer

            // Store the entity in the entity map with the assigned client ID
            std::vector<Entity *> myEntityVector = {entity};
            entityMap[client_id] = myEntityVector;
            // std::cout << "Assigned client ID: " << client_id << ", entity stored in map." << std::endl;
        } else {
            std::cerr << "Failed to extract client ID from server reply!" << std::endl;
        }
    } else {
        std::cerr << "Failed to receive handshake reply from server" << std::endl;
    }
}

// Method to send entity's position data to the server
void Client::sendEntityUpdate() {
    // Send entity's position data to server
    // Send entity's position, size, and color data to the server
    std::string entity_data = std::to_string(client_id) + "," +
                            std::to_string(entity->x) + "," +
                            std::to_string(entity->y) + "," +
                            std::to_string(entity->w) + "," +
                            std::to_string(entity->h) + "," +
                            std::to_string(entity->color.r) + "," +
                            std::to_string(entity->color.g) + "," +
                            std::to_string(entity->color.b) + "," +
                            std::to_string(entity->color.a);

    zmq::message_t message(entity_data.size());
    memcpy(message.data(), entity_data.data(), entity_data.size());

     // Send update to the server from the push socket
    entity_publisher.send(message, zmq::send_flags::none);

    // Wait for server acknowledgment
    // zmq::message_t reply;
    // zmq::recv_result_t result = entity_publisher.recv(reply, zmq::recv_flags::none);
    // if (result) {
    //     std::string reply_str(static_cast<char*>(reply.data()), reply.size());
    //     //std::cout << "Server reply: " << reply_str << std::endl;
    // } else {
    //     //std::cerr << "Failed to receive reply from server" << std::endl;
    // }
}

// Method to receive entity updates from the server
void Client::receiveEntityUpdates() {
    zmq::message_t update_msg;
    std::string broadcast_data = "";

    // Receive entity updates broadcasted from the server
    while (entity_subscriber.recv(update_msg, zmq::recv_flags::dontwait)) {
        broadcast_data = std::string(static_cast<char*>(update_msg.data()), update_msg.size());
    }

    if (strcmp(broadcast_data.c_str(), "") != 0) {

        // Split the broadcasted data into individual entity updates (e.g., "client_id,x,y;")
        std::istringstream iss(broadcast_data);
        std::string entity_update;

        bool serverEntityReceived = false;
        while (std::getline(iss, entity_update, ';')) {
            int received_client_id, x, y, w, h, r, g, b, a;
            sscanf(entity_update.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d", &received_client_id, &x, &y, &w, &h, &r, &g, &b, &a);

            // Ignore updates for the client's own entity
            if (received_client_id != client_id) {

                if (received_client_id == -1) {

                    if (serverEntityReceived) {

                        entityMap[-1].push_back(new Entity(x, y, w, h, {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)}));

                    }
                    else {

                        serverEntityReceived = true;

                        std::vector<Entity *> newEntity = {new Entity(x, y, w, h, {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)})};
                        entityMap[-1] = newEntity;

                    }

                    continue;

                }

                if (entityMap.find(received_client_id) != entityMap.end()) {
                    // Update the entity in the local entityMap

                    entityMap[received_client_id].at(0)->x = x;
                    entityMap[received_client_id].at(0)->y = y;
                    entityMap[received_client_id].at(0)->w = w;
                    entityMap[received_client_id].at(0)->h = h;
                    entityMap[received_client_id].at(0)->color = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)};

                    // std::cout << "Updated entity for client ID " << received_client_id 
                            //   << " to position (" << x << ", " << y << ")" << std::endl;
                } else {
                    // If the entity doesn't exist in the map, add it (optional)
                    std::vector<Entity *> newEntity = {new Entity(x, y, w, h, {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)})};
                    entityMap[received_client_id] = newEntity;
                    // std::cout << "Added new entity for client ID " << received_client_id 
                            //   << " at position (" << x << ", " << y << ")" << std::endl;
                }
            }
        }
    }
}

// Getter for entity_map
unordered_map<int, std::vector<Entity *>>& Client::getEntityMap() {
    return entityMap;
}
