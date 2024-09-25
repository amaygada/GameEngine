#include "serverClient.hpp"

// Default constructor
ServerClient::ServerClient(){};

// Constructs the connection as a server or client, depending on the value of the parameter
ServerClient::ServerClient(std::string type) {

    if (strcmp(type.c_str(), "server") == 0) {

        this->isClientFlag = false;
        this->iterations = {};
        this->connectionID = 0;
        this->numClients = 0;
        this->numEntities = 0;

    }
    else {

        this->isClientFlag = true;
        this->isClientConnected = false;
        this->connectionID = -1;

        // These values are not used for clients
        this->numClients = -1;
        this->numEntities = -1;

    }

    this->context = zmq::context_t(1);

    // Initializes the sockets used for handshake and global commnication
    initSockets();

}

// Updates the connection, server or client using allEntities. allEntities is updated through communication between server and client, then myEntities is updated by looping through allEntities and updating entities in myEntities with matching IDs
void ServerClient::update(std::vector<Entity *> *allEntities, std::vector<Entity *> *myEntities, std::vector<Entity *> *otherEntities) {

    if (this->isClientFlag) {

        // printf("%s", getMessageString(allEntities).c_str());

        printf("T1 %s", getMessageString(allEntities).c_str());

        // Publishes this client's entities to the server (myEntities)
        PUB(myEntities);

        if (this->isClientConnected == false) {

            this->isClientConnected = true;

        }

        printf("T2 %s", getMessageString(allEntities).c_str());

        // Receives all entity updates from the server, updating allEntities
        SUB(allEntities);

        printf("T3 %s\n", getMessageString(allEntities).c_str());

    }
    else {

        // printf("%s", getMessageString(allEntities).c_str());

        // Publishes all entities' information to all clients (allEntities)
        PUB(allEntities);

        // printf("%s", getMessageString(allEntities).c_str());

        // Receives all entity updates from all clients, updating allEntities
        SUB(allEntities);

        // printf("%s\n", getMessageString(allEntities).c_str());

    }

    updateEntities(allEntities, myEntities, otherEntities);

    // if (this->isClientFlag == false) printf("%ld %ld %ld\n", allEntities->size(), myEntities->size(), otherEntities->size());

}

void ServerClient::PUB(std::vector<Entity *> *entities) {

    publishMessage(entities);

}

void ServerClient::SUB(std::vector<Entity *> *entities) {

    subscribeMessage(entities);

}

// Publishes a serialized message of the given list of entities
// The message published by clients follows this format. [Client ID]: [Entity attached client ID] [Entity unique ID] [Entity x] [Entity y] [Entity width] [Entity height] [Entity color.r] [Entity color.g] [Entity color.b] [Entity color.a] [Entity inputHandler boolean] [Entity physicsHandler boolean] [Entity patternHandler boolean], [Continue on like this for each other client entity]\n
// The message published by the server follows this format. [Entity attached client ID] [Entity unique ID] [Entity x] [Entity y] [Entity width] [Entity height] [Entity color.r] [Entity color.g] [Entity color.b] [Entity color.a] [Entity inputHandler boolean] [Entity physicsHandler boolean] [Entity patternHandler boolean], [Continue on like this for each other entity]\n
void ServerClient::publishMessage(std::vector<Entity *> *entities) {

    if (this->isClientFlag == false) {

        // For each entity, if its unique ID has yet to be initialized, initialize it
        const int numEntities = entities->size();
        for (int i = 0; i < numEntities; i++) {

            Entity *entity = entities->at(i);
            if (entity->uniqueID == -1) {

                entity->setUniqueID(this->numEntities + 1);
                this->numEntities++;

            }

        }

    }

    // Get the serialized message
    std::string m = getMessageString(entities);

    // Create a zmq message
    const int messageLength = m.length() + 1;
    zmq::message_t message(messageLength);
    snprintf ((char *) message.data(), messageLength, "%s", m.c_str());
    // printf("%s\n", message.to_string().c_str());

    if (this->isClientFlag) {

        // printf("%s", message.to_string().c_str());
        // Publish the message
        socket_clientupdate.send(message, zmq::send_flags::none);

    }
    else {

        // printf("%s\n", message.to_string().c_str());
        // Publish the message
        socket_serverupdate.send(message, zmq::send_flags::none);

    }

}

// Receives a message from publisher
void ServerClient::subscribeMessage(std::vector<Entity *> *entities) {

    // Variable for storing the message
    zmq::message_t update;

    if (this->isClientFlag) {

        // printf("T2 %s", getMessageString(entities).c_str());

        std::string updateMessage = {};

        while (socket_serverupdate.recv(update, zmq::recv_flags::dontwait)) {

            updateMessage = update.to_string();

            // Just receive the most recent one, don't do anything until after the while loop

        }
        // if (update.to_string().length() == 0) return;
        if (updateMessage.length() == 0) return;
        // printf("%s", updateMessage.c_str());

        std::vector<Entity *> E = getEntitiesFromMessage(updateMessage);
        // printf("%s", getMessageString(entities).c_str());
        // printf("%s\n", getMessageString(&E).c_str());
        std::vector<Entity *> entitiesFromClient = {};

        const int numEntities = E.size();
        for (int i = 0; i < numEntities; i++) {

            Entity *entity = E.at(i);
            // printf("%d ", entity->x);
            const int numOther = entities->size();
            for (int j = 0; j < numOther; j++) {

                Entity *other = entities->at(j);

                if (entity->x == other->x && entity->y == other->y && entity->w == other->w && entity->h == other->h && entity->color.r == other->color.r && entity->color.g == other->color.g && entity->color.b == other->color.b && entity->color.a == other->color.a && ((entity->inputHandler != nullptr && other->inputHandler != nullptr) || (entity->inputHandler == nullptr && other->inputHandler == nullptr)) && ((entity->physicsHandler != nullptr && other->physicsHandler != nullptr) || (entity->physicsHandler == nullptr && other->physicsHandler == nullptr)) && ((entity->patternHandler != nullptr && other->patternHandler != nullptr) || (entity->patternHandler == nullptr && other->patternHandler == nullptr))) { // Good lord

                    // printf("%d, %d, %d\n", entity->attachedClientID, entity->uniqueID, other->uniqueID);

                    // if (this->connectionID == -1) {

                    //     printf("TEST\n");

                    //     entitiesFromClient.push_back(entity);
                    //     break;

                    // }
                    // else {

                    //     // printf("TEST\n");

                    //     other->cloneFields(entity);
                    //     other->uniqueID = entity->uniqueID;
                    //     other->attachedClientID = entity->attachedClientID;

                    //     printf("T2.5 %s", getMessageString(entities).c_str());

                    // }

                    if (this->connectionID == -1) {

                        // printf("TEST\n");

                        entitiesFromClient.push_back(entity);

                    }

                    other->cloneFields(entity);
                    other->uniqueID = entity->uniqueID;
                    other->attachedClientID = entity->attachedClientID;

                    // printf("T2.5 %s", getMessageString(entities).c_str());

                    break;


                }
                else if (j == numOther - 1) {

                    entities->push_back(entity);

                    // printf("T2.75 %s", getMessageString(entities).c_str());

                }

            }

        }

        // printf("%ld\n", entitiesFromClient.size());

        if (this->connectionID == -1 && entitiesFromClient.size() > 0) {

            int ID = entitiesFromClient.at(0)->attachedClientID;
            // printf("%d\n", ID);

            this->connectionID = ID;

        }

        // printf("T3 %s", getMessageString(entities).c_str());

    }
    else {

        std::vector<Entity *> entityUpdates = {};

        int loops = 0;
        while (socket_clientupdate.recv(update, zmq::recv_flags::dontwait)) {
            loops++;
            if (loops > 1) {

                break;

            }

            // printf("%s", update.to_string().c_str());
            std::vector<Entity *> newEntities = getEntitiesFromMessage(update.to_string());
            // printf("%s\n", getMessageString(&newEntities).c_str());

            bool newClient = false;
            
            const int numEntities = newEntities.size();
            const int numUpdates = entities->size();
            // printf("%d\n", numEntities);
            for (int i = 0; i < numEntities; i++) {

                Entity *entity = newEntities.at(i);
                // printf("%d\n", entity->x);
                if (entity->uniqueID == -1) {

                    newClient = true;

                    entity->setUniqueID(this->numEntities + 1);
                    // printf("%d\n", entity->uniqueID);
                    this->numEntities++;

                    entity->setAttachedClientID(this->numClients + 1);

                }

                bool entityFound = false;
                // const int numUpdates = entityUpdates.size();
                // printf("%d\n", numUpdates);
                for (int j = 0; j < numUpdates; j++) {

                    // Entity *other = entityUpdates.at(j);
                    Entity *other = entities->at(j);
                    if (entity->uniqueID == other->uniqueID) {

                        other->cloneFields(entity);
                        
                        entityFound = true;

                    }

                }
                if (numUpdates == 0 || entityFound == false) {

                    // printf("%d\n", entity->uniqueID);
                    entityUpdates.push_back(entity);

                }

            }

            if (newClient) {

                this->numClients++;

            }

        }

        const int numEntityUpdates = entityUpdates.size();
        const int numAllEntities = entities->size();
        // printf("ONE: %sTWO: %s\n", getMessageString(&entityUpdates).c_str(), getMessageString(entities).c_str());
        // printf("%d ", numEntityUpdates);
        for (int i = 0; i < numEntityUpdates; i++) {

            Entity *entityUpdate = entityUpdates.at(i);
            // printf("%d\n", entityUpdate->uniqueID);

            // printf("%d\n", numAllEntities);
            // printf("%s", getMessageString(entities).c_str());
            for (int j = 0; j < numAllEntities; j++) {

                Entity *oldEntity = entities->at(j);
                // printf("%d %d\n", entityUpdate->uniqueID, oldEntity->uniqueID);
                // printf("%d\n", oldEntity->uniqueID);

                if (entityUpdate->uniqueID == oldEntity->uniqueID) {

                    oldEntity->cloneFields(entityUpdate);

                    break;

                }
                else if (j == numAllEntities - 1) {

                    // printf("%d\n", entityUpdate->uniqueID);
                    // printf("%d\n", entityUpdate->attachedClientID);
                    // printf("%d %d\n", i, numEntityUpdates);
                    entities->push_back(entityUpdate);
                    break;

                }

            }

        }

        // printf("\n");

    }

}

// Returns the given 
// The message published by clients follows this format. [Client ID]: [Entity attached client ID] [Entity unique ID] [Entity x] [Entity y] [Entity width] [Entity height] [Entity color.r] [Entity color.g] [Entity color.b] [Entity color.a] [Entity inputHandler boolean] [Entity physicsHandler boolean] [Entity patternHandler boolean], [Continue on like this for each other client entity]\n
// The message published by the server follows this format. [Entity attached client ID] [Entity unique ID] [Entity x] [Entity y] [Entity width] [Entity height] [Entity color.r] [Entity color.g] [Entity color.b] [Entity color.a] [Entity inputHandler boolean] [Entity physicsHandler boolean] [Entity patternHandler boolean], [Continue on like this for each other entity]\n
std::string ServerClient::getMessageString(std::vector<Entity *> *entities) {

    std::string message = "";

    if (this->isClientFlag) {

        message.append(std::to_string(this->connectionID));
        message.append(": ");

    }

    const int numEntities = entities->size();
    for (int i = 0; i < numEntities; i++) {

        Entity *entity = entities->at(i);

        message.append(std::to_string(entity->attachedClientID));
        message.append(" ");

        message.append(std::to_string(entity->uniqueID));
        message.append(" ");

        // if (this->isClientFlag == false) printf("%d ", entity->x);
        message.append(std::to_string(entity->x));
        message.append(" ");

        message.append(std::to_string(entity->y));
        message.append(" ");

        message.append(std::to_string(entity->w));
        message.append(" ");

        message.append(std::to_string(entity->h));
        message.append(" ");

        message.append(std::to_string(entity->color.r));
        message.append(" ");

        message.append(std::to_string(entity->color.g));
        message.append(" ");

        message.append(std::to_string(entity->color.b));
        message.append(" ");

        message.append(std::to_string(entity->color.a));
        message.append(" ");

        message.append(entity->inputHandler == nullptr ? "false " : "true ");

        message.append(entity->physicsHandler == nullptr ? "false " : "true ");

        message.append(entity->patternHandler == nullptr ? "false" : "true");

        if (i < numEntities - 1) {

            message.append(", ");

        }
        else {

            message.append("\n");

        }

    }

    // printf("\n");

    return message;

}

std::vector<Entity *> ServerClient::getEntitiesFromMessage(std::string message) {

    std::vector<Entity *> entities = {};

    int tokenIndex = 0;
    std::string substring = message;
    std::string sub;

    if (this->isClientFlag == false) {

        tokenIndex = substring.find(": ");
        sub = substring.substr(0, tokenIndex);
        std::stoi(sub);
        substring = substring.substr(tokenIndex + 2);

    }

    while (true) {

        // printf("TEST 1 %s\n", substring.c_str());

        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        int attachedClientID = std::stoi(sub);
        // printf("TEST 2\n");

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        int uniqueID = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        int x = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        int y = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        int w = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        int h = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        Uint8 color_r = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        Uint8 color_g = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        Uint8 color_b = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        Uint8 color_a = std::stoi(sub);

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        bool inputHandlerFlag = strcmp(sub.c_str(), "true") == 0 ? true : false;

        substring = substring.substr(tokenIndex + 1);
        tokenIndex = substring.find(" ");
        sub = substring.substr(0, tokenIndex);
        bool physicsHandlerFlag = strcmp(sub.c_str(), "true") == 0 ? true : false;

        substring = substring.substr(tokenIndex + 1);
        // printf("%s\n", substring.c_str());
        tokenIndex = substring.find("e");
        sub = substring.substr(0, tokenIndex + 1);
        bool patternHandlerFlag = strcmp(sub.c_str(), "true") == 0 ? true : false;

        Entity *entity = new Entity(x, y, w, h, {color_r, color_g, color_b, color_a});
        entity->attachedClientID = attachedClientID;
        entity->uniqueID = uniqueID;
        if (inputHandlerFlag) {

            class TempInputHandler : public ModularInputHandler {
                public:
                    void handleInput(Entity *entity) override {
                        // No implementation needed
                    }
            };

            entity->inputHandler = new TempInputHandler();
            
        }
        if (physicsHandlerFlag) {

            class TempPhysicsHandler : public ModularPhysicsHandler {
                public: 
                    void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
                        // No implementation needed
                    }
            };

            entity->physicsHandler = new TempPhysicsHandler();
            
        }
        if (patternHandlerFlag) {

            class TempPatternHandler : public ModularPatternHandler {
                public: 
                    void moveToPath(Entity *entity, int factor) {
                        // No implementation needed
                    }
            };

            entity->patternHandler = new TempPatternHandler();
            
        }

        entities.push_back(entity);

        // printf("%s", substring.c_str());
        substring = substring.substr(tokenIndex + 1);
        // printf("TEST %s\n", substring.c_str());
        tokenIndex = substring.find(", ");
        // printf("%s\n", substring.c_str());
        if (tokenIndex == (int)(std::string::npos)) {

            break;

        }
        else {

            substring = substring.substr(tokenIndex + 2);

        }

    }

    return entities;

}

// Returns boolean representing if this connection is a client or not
bool ServerClient::isClient() {

    return this->isClientFlag;

}

// Returns the ID of this connection
int ServerClient::getConnectionID() {

    return this->connectionID;

}

// Returns the number of clients connected to the server
int ServerClient::getNumClients() {

    return this->iterations.size();

}

// Initializes the sockets used for handshake and global communication
void ServerClient::initSockets() {

    if (this->isClientFlag) {

        this->socket_clientupdate = zmq::socket_t(context, zmq::socket_type::pub);
        this->socket_clientupdate.connect("tcp://localhost:5555");

        this->socket_serverupdate = zmq::socket_t(context, zmq::socket_type::sub);
        this->socket_serverupdate.connect("tcp://localhost:5556");
        const char *filter = "";
        this->socket_serverupdate.setsockopt(ZMQ_SUBSCRIBE, filter, strlen(filter));

    }
    else {

        this->socket_clientupdate = zmq::socket_t(context, zmq::socket_type::sub);
        this->socket_clientupdate.bind("tcp://*:5555");
        const char *filter = "";
        this->socket_clientupdate.setsockopt(ZMQ_SUBSCRIBE, filter, strlen(filter));

        this->socket_serverupdate = zmq::socket_t(context, zmq::socket_type::pub);
        this->socket_serverupdate.bind("tcp://*:5556");

    }

}

// Returns the entity in the vector of the given ID
Entity *ServerClient::getEntityByUniqueID(int ID, std::vector<Entity *> *entities) {

    const int numEntities = entities->size();
    for (int i = 0; i < numEntities; i++) {

        Entity *entity = entities->at(i);
        if (entity->uniqueID == ID) {

            return entity;

        }

    }

    return nullptr;

}

// Updates the myEntities and otherEntities vectors with entities from allEntities
void ServerClient::updateEntities(std::vector<Entity *> *allEntities, std::vector<Entity *> *myEntities, std::vector<Entity *> *otherEntities) {

    // printf("A %s\nB %s\nC %s\n\n", getMessageString(allEntities).c_str(), getMessageString(myEntities).c_str(), getMessageString(otherEntities).c_str());

    const int numEntities = allEntities->size();
    for (int i = 0; i < numEntities; i++) {

        Entity *entity = allEntities->at(i);
        Entity *foundEntity = getEntityByUniqueID(entity->uniqueID, myEntities);

        if (foundEntity == nullptr) foundEntity = getEntityByUniqueID(entity->uniqueID, otherEntities);

        if (foundEntity == nullptr) {

            if (this->isClientFlag && entity->attachedClientID == -1) {

                myEntities->push_back(entity);

            }
            else if (entity->attachedClientID == this->connectionID) {

                myEntities->push_back(entity);

            }
            else {

                otherEntities->push_back(entity);

            }

        }
        else {

            foundEntity->cloneFields(entity);

        }

    }

}

