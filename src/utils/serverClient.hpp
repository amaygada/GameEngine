#include "entity.hpp"

#include <vector>
#include <zmq.hpp>

class ServerClient {

    public:
        ServerClient();

        ServerClient(std::string type);

        void update(std::vector<Entity *> *allEntities, std::vector<Entity *> *myEntities, std::vector<Entity *> *otherEntities);

        bool isClient();

        int getConnectionID();

        int getNumClients();

        std::string getMessageString(std::vector<Entity *> *entities);

    private: 
        // Client-specific fields

        bool isClientFlag;
        bool isClientConnected;

        // Server-specific fields

        std::vector<int> iterations;
        int numClients;
        int numEntities;

        // Common fields

        int connectionID;

        zmq::context_t context;
        zmq::socket_t socket_clientupdate;
        zmq::socket_t socket_serverupdate;

        void initSockets();

        void PUB(std::vector<Entity *> *entities);
        void publishMessage(std::vector<Entity *> *entities);

        void SUB(std::vector<Entity *> *entities);
        void subscribeMessage(std::vector<Entity *> *entities);

        // Helper functions

        std::vector<Entity *> getEntitiesFromMessage(std::string message);
        void updateEntities(std::vector<Entity *> *allEntities, std::vector<Entity *> *myEntities, std::vector<Entity *> *otherEntities);
        Entity *getEntityByUniqueID(int ID, std::vector<Entity *> *entities);

};

