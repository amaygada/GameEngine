#include "entity.hpp"

#include <vector>
#include <zmq.hpp>

struct serializedEntity {

    int length;
    const char *message;

};
typedef struct serializedEntity serializedEntity;

class ServerClient {

    public:
        ServerClient();

        ServerClient(const char *type);

        void serverAddEntities(std::vector<Entity *> entities);

        std::vector<Entity *> update(std::vector<Entity *> clientEntities);

        bool isClient();

        int getClientID();

        int getNumClients();

    private: 
        bool isClientFlag;
        int clientID;
        int numClients;
        std::vector<Entity *> allEntities;

        zmq::context_t context;
        zmq::socket_t req_socket;
        zmq::socket_t pub_socket;

        void initSocket(zmq::socket_t *socket, zmq::socket_type type, const char *port);

        void REQ(std::vector<Entity *> clientEntities);
        void sendToServer(std::vector<Entity *> clientEntities);
        void receiveFromServer(std::vector<Entity *> clientEntities);

        std::vector<Entity *> REP();
        void receiveFromClient(int *clientID);
        void replyToClient(int *clientID);
        
        void PUB();

        std::vector<Entity *> SUB(std::vector<Entity *> clientEntities);

        serializedEntity serializeEntity(Entity *entity);
        std::vector<Entity *> deserializeEntities(std::string message, int *clientID, bool isReqRep);

        std::vector<Entity *> findEntityByID(int ID);

};
