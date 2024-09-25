// #include "server.hpp"

// // Constructor definition
// Server::Server(std::vector<Entity>& entities_ref) : 
//     context(1), 
//     handshake_responder(context, ZMQ_REP), 
//     entity_publisher(context, ZMQ_PUB),
//     entity_responder(context, ZMQ_REP),  // REP socket for entity updates
//     entities(entities_ref)
// {
//     // Bind for handshake (REQ-REP)
//     handshake_responder.bind("tcp://*:5555");

//     // Bind for receiving entity updates (REQ-REP)
//     entity_responder.bind("tcp://*:5556"); 

//     // Bind for broadcasting entity data (PUB-SUB)
//     entity_publisher.bind("tcp://*:5557");
// }

// // Perform the handshake with the client
// void Server::performHandshakeServer() {
//     zmq::message_t request;
//     while (handshake_responder.recv(request, zmq::recv_flags::none)) {
//         std::string client_msg(static_cast<char*>(request.data()), request.size());
//         std::cout << "Handshake received from: " << client_msg << std::endl;

//         // Respond with an acknowledgment
//         std::string reply = "Handshake OK";
//         zmq::message_t reply_msg(reply.size());
//         memcpy(reply_msg.data(), reply.data(), reply.size());
//         handshake_responder.send(reply_msg, zmq::send_flags::none);
//     }
// }

// // Receive updates from clients and update the entities vector
// void Server::receiveEntityUpdates() {
//     zmq::message_t request;
//     if (entity_responder.recv(request, zmq::recv_flags::none)) {
//         std::string entity_data(static_cast<char*>(request.data()), request.size());
//         std::cout << "Received entity update: " << entity_data << std::endl;

//         // Parse the received data and update the entities vector
//         int id, x, y;
//         sscanf(entity_data.c_str(), "%d,%d,%d", &id, &x, &y);

//         if (id >= 0 && static_cast<std::size_t>(id) < entities.size()) {
//             entities[id].x = x;
//             entities[id].y = y;
//         } else {
//             std::cerr << "Invalid entity ID received: " << id << std::endl;
//         }

//         // Respond with acknowledgment
//         std::string reply = "Update received";
//         zmq::message_t reply_msg(reply.size());
//         memcpy(reply_msg.data(), reply.data(), reply.size());
//         entity_responder.send(reply_msg, zmq::send_flags::none);
//     }
// }

// // Broadcast updated entity positions to all clients
// void Server::broadcastEntityUpdates() {
//     // Prepare updated entity data to broadcast
//     std::string broadcast_message;
//     for (size_t i = 0; i < entities.size(); ++i) {
//         broadcast_message += std::to_string(i) + "," + 
//                              std::to_string(entities[i].x) + "," + 
//                              std::to_string(entities[i].y) + ";";
//     }

//     zmq::message_t message(broadcast_message.size());
//     memcpy(message.data(), broadcast_message.data(), broadcast_message.size());
//     entity_publisher.send(message, zmq::send_flags::none);
// }

#include "server.hpp"

// Constructor definition
Server::Server(std::vector<Entity>& entities_ref) : 
    context(1), 
    handshake_responder(context, ZMQ_REP), 
    entity_publisher(context, ZMQ_PUB),
    pull_socket(context, ZMQ_PULL),  
    entities(entities_ref)
    {
        // Bind for handshake (REQ-REP)
        handshake_responder.bind("tcp://*:5555");

        // Bind for receiving updates from clients (PULL)
        pull_socket.bind("tcp://*:5556");

        // Bind for broadcasting entity data (PUB-SUB)
        entity_publisher.bind("tcp://*:5557");

    }

void Server::handleClient(int client_id) {

    // Receive entity updates from this client
    zmq::message_t request;
    while (true) {
        if (pull_socket.recv(request, zmq::recv_flags::dontwait)) {
            std::string entity_data(static_cast<char*>(request.data()), request.size());
            //std::cout << "Received entity update from client " << client_id << ": " << entity_data << std::endl;

            // Parse the received data and update the entities vector
            int id, x, y;
            sscanf(entity_data.c_str(), "%d,%d,%d", &id, &x, &y);

            {
                // Lock mutex to protect access to entities vector
                std::lock_guard<std::mutex> lock(entity_mutex);
                if (id >= 0 && static_cast<std::size_t>(id) < entities.size()) {
                    entities[id].x = x;
                    entities[id].y = y;
                } else {
                    std::cerr << "Invalid entity ID received from client " << client_id << ": " << id << std::endl;
                }
            }

            // Respond with acknowledgment
            // std::string reply = "Update received";
            // zmq::message_t reply_msg(reply.size());
            // memcpy(reply_msg.data(), reply.data(), reply.size());
            // entity_responder.send(reply_msg, zmq::send_flags::none);
        }
    }
}

void Server::run() {
    int client_id = 0;
    std::vector<std::thread> client_threads;

    while (true) {
        std::cout<< "Debug inside while loop" << std::endl;
        // Wait for new clients (this could be a new handshake request)
        zmq::message_t request;
        if (handshake_responder.recv(request, zmq::recv_flags::none)) {
            std::string client_msg(static_cast<char*>(request.data()), request.size());
            std::cout << "Handshake received from client " << client_id << ": " << client_msg << std::endl;

            // Respond with an acknowledgment
            std::string reply = "Handshake OK for client " + std::to_string(client_id);
            zmq::message_t reply_msg(reply.size());
            memcpy(reply_msg.data(), reply.data(), reply.size());
            handshake_responder.send(reply_msg, zmq::send_flags::none);

            // Create a new thread for the client, pass the correct client ID
            client_threads.push_back(std::thread(&Server::handleClient, this, client_id));

            // Increment client_id safely after creating the thread
            client_id++;
        }

        // Broadcast updated entity positions to all clients
        broadcastEntityUpdates();
        //std::cout << "After bc" << client_id << std::endl;
    }

    // Join all client threads before exiting
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Server::broadcastEntityUpdates() {
    std::string broadcast_message;
    std::cout << broadcast_message << std::endl;

    // Prepare updated entity data to broadcast
    {
        std::lock_guard<std::mutex> lock(entity_mutex);
        for (size_t i = 0; i < entities.size(); ++i) {
            broadcast_message += std::to_string(i) + "," + 
                                 std::to_string(entities[i].x) + "," + 
                                 std::to_string(entities[i].y) + ";";
        }
    }

    zmq::message_t message(broadcast_message.size());
    memcpy(message.data(), broadcast_message.data(), broadcast_message.size());
    entity_publisher.send(message, zmq::send_flags::none);
}
