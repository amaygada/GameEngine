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
        } else if (value.m_Type == value.TYPE_ENTITYMAP) {
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
        } else {
            if (value.m_Type == value.TYPE_INT) {
                message.append(std::to_string( (int)(value.m_asInt) ));
            }
            else if (value.m_Type == value.TYPE_FLOAT) {
                message.append(std::to_string( (float)(value.m_asFloat) ));
            }
            else if (value.m_Type == value.TYPE_DOUBLE) {
                message.append(std::to_string( (double)(value.m_asDouble) ));
            }
        }
        message.append("], "); 
    }
    message.append("; ");
    return message;
}

// Helper function for parsing the game object (assuming details on game object format)
Entity* parseGameObject(const std::string& valueData) {
    // Parse and recreate the Entity (stubbed here)
    return new Entity();
}

Event* Serializer::deserializeEvent(const std::string& serializedMessage) {
    std::istringstream stream(serializedMessage);
    std::string segment;
    
    int time;
    std::string eventType;
    std::map<std::string, variant> parameters;

    // Parse time
    if (std::getline(stream, segment, ';')) {
        size_t pos = segment.find("Time: ");
        if (pos != std::string::npos) {
            try {
                time = std::stoi(segment.substr(pos + 6));
            } catch (...) {
                std::cerr << "Error parsing time" << std::endl;
                return nullptr;
            }
        }
    }

    // Parse event type
    if (std::getline(stream, segment, ';')) {
        size_t pos = segment.find("Event type: ");
        if (pos != std::string::npos) {
            eventType = segment.substr(pos + 12);
        }
    }

    // Parse parameters
    if (std::getline(stream, segment, ';')) {
        size_t pos = 0;
        while ((pos = segment.find("[Parameter type: ")) != std::string::npos) {
            size_t endPos = segment.find("], ", pos);
            if (endPos == std::string::npos) break;

            std::string paramSegment = segment.substr(pos + 17, endPos - (pos + 17));
            size_t typePos = paramSegment.find(", Value type: ");
            if (typePos == std::string::npos) break;

            std::string paramName = paramSegment.substr(0, typePos);
            std::string valueSegment = paramSegment.substr(typePos + 14);

            int valueType;
            size_t valueTypeEnd = valueSegment.find_first_of(", ");
            if (valueTypeEnd == std::string::npos) break;

            try {
                valueType = std::stoi(valueSegment.substr(0, valueTypeEnd));
            } catch (...) {
                std::cerr << "Error parsing value type" << std::endl;
                return nullptr;
            }

            size_t valueStart = valueSegment.find(", Value: ") + 9;
            if (valueStart == std::string::npos) break;

            std::string valueData = valueSegment.substr(valueStart);
            if (valueType == variant::TYPE_INT) {
                parameters[paramName] = variant(std::stoi(valueData));
            } else if (valueType == variant::TYPE_FLOAT) {
                parameters[paramName] = variant(std::stof(valueData));
            } else if (valueType == variant::TYPE_DOUBLE) {
                parameters[paramName] = variant(std::stod(valueData));
            } else if (valueType == variant::TYPE_GAMEOBJECT) {
                parameters[paramName] = variant(parseGameObject(valueData));
            }

            segment.erase(0, endPos + 3);
        }
    }

    Event* event = new Event(eventType);
    event->parameters = parameters;
    return event;
}