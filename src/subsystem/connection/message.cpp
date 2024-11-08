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
    return "";
}

string MessageHandler::receiveMessage(zmq::socket_t &sock) {
    zmq::message_t reply;
    if(sock.recv(reply, zmq::recv_flags::none)){
        string message = string(static_cast<char*>(reply.data()), reply.size());
        return message;
    }
    return "";
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

std::string Serializer::serializeEvent(Event *event, int time) {

    string message = "Time: " + std::to_string(time) + 
                     "; Event type: " + event->type + 
                     "; Parameters: ";
    std::map<std::string, variant> parameters = event->parameters;
    for (pair<std::string, variant> p : parameters) {

        std::string type = p.first;
        variant value = p.second;

        message.append("[Parameter type: " + type + ", Value type: " + std::to_string(value.m_Type) + ", Value: ");

        if (value.m_Type == value.TYPE_GAMEOBJECT) {

            Entity *entity = value.m_asGameObject;
            std::string entitySerial = serializeEntity(entity);

            message.append("(" + entitySerial + ")");

        }
        else if (value.m_Type == value.TYPE_ENTITYMAP) {

            message.append("{");

            std::unordered_map<int, std::vector<Entity *>> entities = *(value.m_asEntityMap);
            for (pair<int, std::vector<Entity *>> list : entities) {

                int ID = list.first;
                std::vector<Entity *> E = list.second;
                message.append(std::to_string(ID) + ", ");
                for (Entity *entity : E) {

                    std::string entitySerial = serializeEntity(entity);
                    message.append("(" + entitySerial + "), ");

                }

            }

            message.append("}");

        }
        else {

            if (value.m_Type == value.TYPE_INT) {

                message.append(std::to_string( (int)(value.m_asInt) ));

            }
            else if (value.m_Type == value.TYPE_FLOAT) {

                message.append(std::to_string( (float)(value.m_asFloat) ));

            }
            else if (value.m_Type == value.TYPE_DOUBLE) {

                message.append(std::to_string( (double)(value.m_asDouble) ));

            }
            // No support for timelines, this would be too complex to implement

        }

        message.append("], "); 

    }

    message.append("; ");

    printf("%s\n", message.c_str());
    return message;

}

std::string Serializer::serializeEntity(Entity *entity) {
    string request_message = "Entity data: x=" + std::to_string(entity->x) +
                                ", y=" + std::to_string(entity->y) +
                                ", w=" + std::to_string(entity->w) +
                                ", h=" + std::to_string(entity->h) +
                                ", r=" + std::to_string(entity->color.r) +
                                ", g=" + std::to_string(entity->color.g) +
                                ", b=" + std::to_string(entity->color.b) +
                                ", a=" + std::to_string(entity->color.a) + 
                                ", name=" + entity->getName();
    return request_message;
}

Entity* Serializer::deserializeEntity(std::string data) {
    int x, y, w, h, r, g, b, a;
    char name[256];
    sscanf(data.c_str(), "Entity data: x=%d, y=%d, w=%d, h=%d, r=%d, g=%d, b=%d, a=%d, name=%255[^\n]", &x, &y, &w, &h, &r, &g, &b, &a, &name);
    SDL_Color color = {r, g, b, a};
    Entity *entity = new Entity(x, y, w, h, color);
    entity->setName(name);
    return entity;
}