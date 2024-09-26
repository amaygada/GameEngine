#include "server.hpp"

// Constructor definition
Server::Server() : 
    context(1), 
    handshake_responder(context, ZMQ_REP), 
    entity_publisher(context, ZMQ_PUB),
    entity_responder(context, ZMQ_PULL)
    {
        // Bind for handshake (REQ-REP)
        handshake_responder.bind("tcp://*:5555");

        // Bind for receiving updates from clients (PULL)
        entity_responder.bind("tcp://*:5556");

        // Bind for broadcasting entity data (PUB-SUB)
        entity_publisher.bind("tcp://*:5557");

    }

void Server::handleClient(int client_id) {
    zmq::message_t request;

    while (true) {
        {
            // Lock mutex to protect access to entityMap
            std::lock_guard<std::mutex> lock(entity_mutex);

            // Receive entity update from this client
            if (entity_responder.recv(request, zmq::recv_flags::dontwait)) {
                std::string entity_data(static_cast<char*>(request.data()), request.size());
                // std::cout << "Received entity update from client " << client_id << ": " << entity_data << std::endl;

                // Parse the received data (we expect the data to be "client_id,x,y,w,h,r,g,b,a")
                int received_id, x, y, w, h, r, g, b, a;
                sscanf(entity_data.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d", &received_id, &x, &y, &w, &h, &r, &g, &b, &a);

                // Check if the received client ID matches the current client_id
                if (received_id == client_id) {
                    // Update the entity in the entityMap
                    if (entityMap.find(client_id) != entityMap.end()) {
                        entityMap[client_id]->x = x;
                        entityMap[client_id]->y = y;
                        entityMap[client_id]->w = w;
                        entityMap[client_id]->h = h;
                        entityMap[client_id]->color = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)};
                    } else {
                        std::cerr << "Entity not found for client " << client_id << std::endl;
                    }
                }
            }

            // Optionally, send an acknowledgment (if required by your design)
            // std::string reply = "Update received";
            // zmq::message_t reply_msg(reply.size());
            // memcpy(reply_msg.data(), reply.data(), reply.size());
            // entity_responder.send(reply_msg, zmq::send_flags::none);
        }
    }
}

void Server::broadcastEntityUpdates() {
    std::string broadcast_message;

    // Prepare updated entity data to broadcast (client_id, x, y for each entity)
    {
        std::lock_guard<std::mutex> lock(entity_mutex);
        // Prepare updated entity data to broadcast (client_id, x, y, w, h, r, g, b, a for each entity)
        for (const auto& pair : entityMap) {
            int client_id = pair.first;
            Entity* entity = pair.second;
            broadcast_message += std::to_string(client_id) + "," +
                                std::to_string(entity->x) + "," +
                                std::to_string(entity->y) + "," +
                                std::to_string(entity->w) + "," +
                                std::to_string(entity->h) + "," +
                                std::to_string(entity->color.r) + "," +
                                std::to_string(entity->color.g) + "," +
                                std::to_string(entity->color.b) + "," +
                                std::to_string(entity->color.a) + ";";
        }
    }

    // Send the broadcast message to all clients
    zmq::message_t message(broadcast_message.size());
    memcpy(message.data(), broadcast_message.data(), broadcast_message.size());
    entity_publisher.send(message, zmq::send_flags::none);
}

void Server::run() {
    std::srand(std::time(nullptr));  // Seed for random client ID generation
    std::vector<std::thread> client_threads;

    while (true) {
        // Wait for new clients (this could be a new handshake request)
        zmq::message_t request;
        if (handshake_responder.recv(request, zmq::recv_flags::dontwait)) {
            std::string client_msg(static_cast<char*>(request.data()), request.size());
            // std::cout << "Received handshake request: " << client_msg << std::endl;

            // std::cout << client_msg << std::endl;
            // Parse entity data (x, y, w, h, and color)
            int x = 0, y = 0, w = 0, h = 0, r = 0, g = 0, b = 0, a = 0;
            sscanf(client_msg.c_str(), "Entity data: x=%d, y=%d, w=%d, h=%d, r=%d, g=%d, b=%d, a=%d", &x, &y, &w, &h, &r, &g, &b, &a);
            // std::cout << "Client entity position: (" << x << ", " << y << ")" << std::endl;

            // Assign a random client ID
            int client_id = rand() % 1000;
            std::string reply = "Handshake OK, assigned client ID: " + std::to_string(client_id);

            // Store the entity data in the server's entityMap
            entityMap[client_id] = new Entity(x, y, w, h, {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)});
            // std::cout << "Server stored entity for client ID " << client_id << std::endl;

            // Respond with the assigned client ID
            zmq::message_t reply_msg(reply.size());
            memcpy(reply_msg.data(), reply.data(), reply.size());
            handshake_responder.send(reply_msg, zmq::send_flags::none);

            // (Optional) Create a thread to handle this client, passing the client_id
            client_threads.push_back(std::thread(&Server::handleClient, this, client_id));
        }

        // Broadcast updated entity positions to all clients
        broadcastEntityUpdates();
    }

    // Join all client threads before exiting
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// Getter for entity_map
unordered_map<int, Entity*> Server::getEntityMap() {
    return entityMap;
}