#include "serverClient.hpp"

ServerClient::ServerClient() {
}

// Construct the connection based on the given type
ServerClient::ServerClient(const char *type) {

    if (strcmp(type, "server") == 0) {

        isClientFlag = false;
        clientID = 0;
        numClients = 0;
        allEntities = {};

        context = zmq::context_t(1);
        initSocket(&req_socket, zmq::socket_type::rep, "tcp://*:5555");
        initSocket(&pub_socket, zmq::socket_type::pub, "tcp://*:5556");

    }
    else {

        isClientFlag = true;
        clientID = -1;
        numClients = -1;
        allEntities = {};
        
        context = zmq::context_t(1);
        initSocket(&req_socket, zmq::socket_type::req, "tcp://localhost:5555");
        initSocket(&pub_socket, zmq::socket_type::sub, "tcp://localhost:5556");

    }

}

// Initializes the given socket
void ServerClient::initSocket(zmq::socket_t *socket, zmq::socket_type type, const char *port) {

    *socket = zmq::socket_t(this->context, type);

    // If this connection is a client...
    if (this->isClientFlag) {

        // Connect the socket to the port
        socket->connect(port);
        // If the connection type is SUB, set the filter to none (aka it will receive all messages from the PUB)
        if (type == zmq::socket_type::sub) {

            const char *filter = "";
            socket->setsockopt(ZMQ_SUBSCRIBE, filter, strlen(filter));

        }

    }
    // Otherwise the connection is a server...
    else {

        // Bind the socket to the port
        socket->bind(port);

    }

}

// Adds the given entities to this component's list of entities - Only if this connection is a server
void ServerClient::serverAddEntities(std::vector<Entity *> entities) {

    if (this->isClientFlag == false) {

        const int numNewEntities = entities.size();
        for (int i = 0; i < numNewEntities; i++) {

            Entity *entity = entities.at(i);
            allEntities.push_back(entity);

        }
        
    }

}

// Returns boolean representing if this connection is a client
bool ServerClient::isClient() {

    return this->isClientFlag;

}

// Returns this client's (unique) ID
int ServerClient::getClientID() {

    return clientID;

}

// Returns the number of connected clients
int ServerClient::getNumClients() {

    return this->numClients;

}

// Serializes the given entity into a struct containing a message and the message's length
serializedEntity ServerClient::serializeEntity(Entity *entity) {

    std::string entityString = std::string();
    entityString.append(std::to_string(entity->x));
    entityString.append(" ");
    entityString.append(std::to_string(entity->y));
    entityString.append(" ");
    entityString.append(std::to_string(entity->w));
    entityString.append(" ");
    entityString.append(std::to_string(entity->h));
    entityString.append(" ");
    entityString.append(std::to_string(entity->color.r));
    entityString.append(" ");
    entityString.append(std::to_string(entity->color.g));
    entityString.append(" ");
    entityString.append(std::to_string(entity->color.b));
    entityString.append(" ");
    entityString.append(std::to_string(entity->color.a));
    entityString.append(" ");
    entityString.append(std::to_string(entity->attachedClientID));
    entityString.append("\n");

    serializedEntity entitySerial = {(int)(entityString.length()), entityString.c_str()};
    
    return entitySerial;

}

// Deserializes a given list of entities into their object forms. This function also sets the given client ID if this is the client's first time connecting
std::vector<Entity *> ServerClient::deserializeEntities(std::string message, int *clientID, bool isReqRep) {

    int startingIndex = 0;

    // If this connection is REQ-REP...
    if (isReqRep) {

        // Index to stop checking from
        int indexID = message.find("\n");
        std::string stringID = message.substr(0, indexID);
        // Temporary ID variable
        int ID = -1;
        // If everything before the first newline character is "C", a new client is connecting
        if (strcmp(stringID.c_str(), "C") == 0) {

            // Update the counter for the number of clients connected to the server
            numClients++;
            // Update the client's ID
            *clientID = numClients;
            ID = *clientID;

        }
        else {

            // Update the temporary ID variable
            ID = std::stoi(stringID);

        }

        // Update the client's ID
        *clientID = ID;
        // Index to stop checking from
        startingIndex = indexID + 1;

        // If this connection is a client...
        if (this->isClientFlag) {

            // Get the substring starting after the newline
            std::string newMessage = message.substr(indexID + 1);
            // The next section of the message should contain the number of clients
            int indexNumClients = message.find("\n");
            // The substring only containing the number of clients
            std::string stringNumClients = newMessage.substr(indexID + 1, indexNumClients - (indexID + 1));
            const int num = std::stoi(stringNumClients);

            // Sets the number of clients
            this->numClients = num;
            // Increments the substring's starting index
            startingIndex = indexNumClients + 1;

        }

    }

    // List of deserialized entities
    std::vector<Entity *> entities = {};

    // An entity's values
    int x, y, w, h;
    Uint8 color_r, color_g, color_b, color_a;
    int ID;

    // While the index is valid...
    while (startingIndex >= 0 && startingIndex < ((int)(message.length())) - 1 && startingIndex != (int)(std::string::npos)) {

        // Get the substring starting from the index
        std::string subStr = message.substr(startingIndex);

        // Keep reading in values, which are separated by spaces

        // Index of the next space
        int spaceIndex = subStr.find(" ");
        // Get the substring containing only this value
        std::string temp = subStr.substr(0, spaceIndex);
        // Store the value
        x = std::stoi(temp);
        // Update the substring to move past this value
        subStr = subStr.substr(spaceIndex + 1);

        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        y = std::stoi(temp);
        subStr = subStr.substr(spaceIndex + 1);

        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        w = std::stoi(temp);
        subStr = subStr.substr(spaceIndex + 1);

        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        h = std::stoi(temp);
        subStr = subStr.substr(spaceIndex + 1);

        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        color_r = std::stoi(temp);
        subStr = subStr.substr(spaceIndex + 1);

        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        color_g = std::stoi(temp);
        subStr = subStr.substr(spaceIndex + 1);

        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        color_b = std::stoi(temp);
        subStr = subStr.substr(spaceIndex + 1);

        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        color_a = std::stoi(temp);
        subStr = subStr.substr(spaceIndex + 1);
        
        spaceIndex = subStr.find(" ");
        temp = subStr.substr(0, spaceIndex);
        ID = std::stoi(temp);

        // Construct the entity from its deserialized values
        SDL_Color color = {color_r, color_g, color_b, color_a};
        Entity *entity = new Entity(x, y, w, h, color);
        entity->setID(ID);

        // Push the entity to the vector
        entities.push_back(entity);

        // Set the index to the next entity's data
        startingIndex = subStr.find("\n") + 1;
            
    }

    return entities;

}

// Sends the vector of client-side entities to the server
void ServerClient::sendToServer(std::vector<Entity *> clientEntities) {

    // Number of client-side entities
    const int numClientEntities = clientEntities.size();
    // Vector of serialized client-side entities
    std::vector<serializedEntity> serializedEntities = {};
    // Size field used when creating the request string
    int sizeOfClientEntities = 0;
    for (int i = 0; i < numClientEntities; i++) {

        Entity *entity = clientEntities.at(i);
        serializedEntity entitySerial = serializeEntity(entity);
        serializedEntities.push_back(entitySerial);

        sizeOfClientEntities += entitySerial.length;

    }

    // Create the request string based on the size of the entities + any starting information
    zmq::message_t request(2 + sizeOfClientEntities);
    int dataAlign = 0;

    // If this is the client's first time connecting...
    if (clientID == -1) {

        // Start the message off with "C"
        memcpy(request.data(), "C\n", 2);
        dataAlign = 2;

    }
    // Otherwise...
    else {

        // Start the message with the client's ID
        std::string message = std::to_string(clientID);
        message.append("\n");

        memcpy(request.data(), message.c_str(), message.length());
        dataAlign = message.length();

    }

    // For each client-side entity...
    for (int i = 0; i < numClientEntities; i++) {

        // Get the serialized entity
        serializedEntity entitySerial = serializedEntities.at(i);
        // The serialized entity's message's length
        const int messageLength = entitySerial.length;
        // The serialized entity's message
        const char *message = entitySerial.message;

        // Store the message's information
        memcpy((char*)(request.data()) + dataAlign, message, messageLength);
        dataAlign += messageLength;

    }

    // Send the message to the server
    req_socket.send(request, zmq::send_flags::none);

}

// Receives a reply from the server
void ServerClient::receiveFromServer(std::vector<Entity *> clientEntities) {

    zmq::message_t reply;
    // If something went wrong while receiving the message, throw an exception
    if (!(req_socket.recv(reply, zmq::recv_flags::none))) {

        throw new exception();

    }

    // Deserialize the entities. The only purpose of doing this here is to ensure the client's ID gets set if this is its first time connecting
    deserializeEntities(reply.to_string(), &clientID, true);

}

// Performs the REQ side of REQ-REP connection
void ServerClient::REQ(std::vector<Entity *> clientEntities) {

    sendToServer(clientEntities);
    receiveFromServer(clientEntities);

}

// Performs the SUB side of PUB-SUB connection
std::vector<Entity *> ServerClient::SUB(std::vector<Entity *> clientEntities) {

    zmq::message_t update;
    // If something went wrong while receiving the message, throw an exception
    if (!(pub_socket.recv(update, zmq::recv_flags::none))) {

        throw new exception();

    }

    // Store the entities received from the published message
    std::vector<Entity *> response = deserializeEntities(update.to_string(), &clientID, false);
    // Number of entities
    const int numEntities = response.size();
    // Clear the stored list of all entities to replace it with the published information by the server
    this->allEntities.clear();
    // For each entity, push it to the stored list of all entities
    for (int i = 0; i < numEntities; i++) {

        this->allEntities.push_back(response.at(i));

    }

    return allEntities;

}

// Returns a vector of any entities attached to the given client ID
std::vector<Entity *> ServerClient::findEntityByID(int ID) {

    std::vector<Entity *> clientEntities = {};

    const int numEntities = allEntities.size();
    for (int i = 0; i < numEntities; i++) {

        Entity *entity = allEntities.at(i);
        if (entity->attachedClientID == ID) {

            clientEntities.push_back(entity);

        }

    }

    return clientEntities;

}

// Receives a message request from a client with the given ID
void ServerClient::receiveFromClient(int *clientID) {

    zmq::message_t request;
    // If something went wrong while receiving the message, throw an exception
    if (!(req_socket.recv(request, zmq::recv_flags::none))) {

        throw new exception();

    }

    // If this is the client's first time connecting...
    if (request.to_string().at(0) == 'C') {

        // Deserialize the entities
        std::vector<Entity *> newEntities = deserializeEntities(request.to_string(), clientID, true);
        const int numNewEntities = newEntities.size();
        // For each entity...
        for (int i = 0; i < numNewEntities; i++) {

            Entity *newEntity = newEntities.at(i);
            // Set its attached client ID
            newEntity->setID(*clientID);

            // Add it to the stored list of all entities
            allEntities.push_back(newEntity);

        }

    }
    // Otherwise if it is not the client's first time connecting...
    else {
        
        // Store the client ID
        int stopIndex = request.to_string().find('\n');
        std::string stringID = request.to_string().substr(0, stopIndex); 

        *clientID = std::stoi(stringID);

        std::vector<Entity *> clientEntities = findEntityByID(*clientID);
        std::vector<Entity *> newEntities = deserializeEntities(request.to_string(), clientID, true);

        const int numEntities = newEntities.size();
        int currentClientEntity = 0;
        for (int i = 0; i < numEntities; i++) {

            Entity *newEntity = newEntities.at(i);
            if (newEntity->attachedClientID == *clientID) {

                Entity *clientEntity = clientEntities.at(currentClientEntity);
                currentClientEntity++;

                *clientEntity = *newEntity;

            }

        }

    }

}

// Sends a reply to the client with the given ID
void ServerClient::replyToClient(int *clientID) {

    // Number of entities in the server-side (global total of entities)
    const int numEntities = allEntities.size();
    std::vector<serializedEntity> serializedEntities = {};
    int sizeOfAllEntities = 0;
    for (int i = 0; i < numEntities; i++) {

        Entity *entity = allEntities.at(i);
        serializedEntity entitySerial = serializeEntity(entity);
        serializedEntities.push_back(entitySerial);

        sizeOfAllEntities += entitySerial.length;

    }

    // Construct the reply
    std::string stringID = std::to_string(*clientID);
    stringID.append("\n");
    stringID.append(std::to_string(numClients));
    stringID.append("\n");

    zmq::message_t reply(stringID.length() + sizeOfAllEntities);
    int dataAlign = 0;

    memcpy(reply.data(), stringID.c_str(), stringID.length());
    dataAlign = stringID.length();

    for (int i = 0; i < numEntities; i++) {

        serializedEntity entitySerial = serializedEntities.at(i);
        const int messageLength = entitySerial.length;
        const char *message = entitySerial.message;

        memcpy((char *)(reply.data()) + dataAlign, message, messageLength);
        dataAlign += messageLength;

    }

    // Send the reply to the client
    req_socket.send(reply, zmq::send_flags::none);

}

// Performs the REP side of REQ-REP connection
std::vector<Entity *> ServerClient::REP() {

    // Client ID. Starts off at an invalid value, but is updated when the client connects in receiveFromClient()
    int clientID = -1;

    receiveFromClient(&clientID);
    replyToClient(&clientID);

    return allEntities;

}

// Performs the PUB side of PUB-SUB connection
void ServerClient::PUB() {

    const int numEntities = allEntities.size();
    std::vector<serializedEntity> serializedEntities = {};
    int sizeOfClientEntities = 0;
    for (int i = 0; i < numEntities; i++) {

        Entity *entity = allEntities.at(i);
        serializedEntity entitySerial = serializeEntity(entity);
        serializedEntities.push_back(entitySerial);

        sizeOfClientEntities += entitySerial.length;

    }

    zmq::message_t toPublish(sizeOfClientEntities);
    int dataAlign = 0;

    for (int i = 0; i < numEntities; i++) {

        serializedEntity entitySerial = serializedEntities.at(i);
        const int messageLength = entitySerial.length;
        const char *message = entitySerial.message;

        memcpy((char *)(toPublish.data()) + dataAlign, message, messageLength);
        dataAlign += messageLength;

    }

    pub_socket.send(toPublish, zmq::send_flags::none);

}

// Performs routine update for the connection, client or server-side
std::vector<Entity *> ServerClient::update(std::vector<Entity *> clientEntities) {

    // If the connection is a client, perform REQ, then SUB
    if (this->isClientFlag) {

        REQ(clientEntities);

        std::vector<Entity *> response = SUB(clientEntities);

        return response;

    }
    // If the connection is a server, perform REP, then PUB
    else {

        std::vector<Entity *> entityUpdates = REP();

        PUB();

        return allEntities;

    }

}
