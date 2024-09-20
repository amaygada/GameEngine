#include "entity.hpp"

#include <vector>
#include <zmq.hpp>

class ServerClient {

    public:
        ServerClient(char *type);

        bool initSocket(zmq::socket_t socket, zmq::socket_type type);

        bool update(std::vector<Entity> entities);

        bool isClient();

        int numClients();

    private: 
        bool isClient;
        std::vector<Entity> entities;
        zmq::context_t context;
        zmq::socket_t req_socket;
        zmq::socket_t pub_socket;

        bool sendToServer(char *message);
        bool receiveFromClient(char *request);
        bool publishMessage(char *message);
        bool subscribeMessage();

};
