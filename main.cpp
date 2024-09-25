#include "main.hpp"
#include <memory>
#include <vector>

// Returns a vector containing all elements from v1 and v2
std::vector<Entity *> joinVectors(std::vector<Entity *> v1, std::vector<Entity *> v2) {

    std::vector<Entity *> V = v1;
    
    const int num = v2.size();
    for (int i = 0; i < num; i++) {

        V.push_back(v2.at(i));

    }

    return V;

}

int main(int argc, char *argv[]) {

    // The connection, server or client
    ServerClient connection;

    // If the command-line argument is not "server" or there is an invalid number of arguments, initialize the connection as a client
    if (argc != 2 || strcmp(argv[1], "server") != 0) {

        connection = ServerClient(std::string(""));

    }
    // Otherwise, the command-line argument is "server". Initialize the connection as a server
    else {
    
        connection = ServerClient(std::string("server"));

    }

    // Vector to hold all connection-side entities. For clients, this is individual client-side entities. For the server, this is all server-side entities
    std::vector<Entity *> myEntities = {};

    // Vector to hold all server-side entities, including entities from other clients. For clients, this is all entities that do not belong to this client. For the server, this is all entities that do not belong to the server
    std::vector<Entity *> otherEntities = {};

    // If the connection is a client, initialize the window
    if (connection.isClient()) {

        renderer->init("Game");

    }

    if (connection.isClient()) {

        // Create an entity
        SDL_Color shapeColor5 = {0, 0, 255, 255};  // Blue color
        Entity *shape5 = new Entity(300, 300, 100, 100, shapeColor5);
        // shape5.inputHandler = new DefaultEntityInputHandler();
        shape5->physicsHandler = new DefaultMovementPhysicsHandler(true);
        // Do not set the entity's attached client ID here. This will be handled when the server first receives the client's connection
        shape5->setAttachedClientID(-1);
        // Do not set the entity's unique ID here. This will be handled when the server first publishes after receiving the client's connection
        shape5->setUniqueID(-1);
        myEntities.push_back(shape5);

    }
    else {

        SDL_Color shapeColor = {255, 0, 0, 255};  // Red color
        Entity *shape1 = new Entity(1000, 0, 100, 500, shapeColor);
        shape1->physicsHandler = new DefaultGravityPhysicsHandler(false);
        // Set the entity's attached client ID to be that of the server (0)
        shape1->setAttachedClientID(0);
        // Do not set the entity's unique ID here. This will be handled when the server first publishes entities' information
        shape1->setUniqueID(-1);
        myEntities.push_back(shape1);

        // Initialize the pattern-following shape
        SDL_Color shapeColor3 = {255, 255, 0, 255};  // Yellow color
        Entity *shape3 = new Entity( 10, 10, 100, 100, shapeColor3);
        std::vector<SDL_Rect> shape3Path = {};
        shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH-100,10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
        shape3Path.push_back({10, 10, 1, 1});
        shape3->patternHandler = new DefaultPatternHandler(shape3Path);
        shape3->setAttachedClientID(0);
        shape3->setUniqueID(-1);
        myEntities.push_back(shape3);

    }

    int64_t last_render_time = globalTimeline->getTime();

    while (true) {

        int64_t frame_delta = globalTimeline->getTime() - last_render_time;
        
        if(frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);

        last_render_time = globalTimeline->getTime();

        if (connection.isClient()) {

            renderer->prepareScene();

        }

        inputSubsystem->doInput(myEntities);

        std::vector<Entity *> allEntities = joinVectors(otherEntities, myEntities);

        // printf("%s", connection.getMessageString(&allEntities).c_str());
        // printf("%ld\n", allEntities.size());
        connection.update(&allEntities, &myEntities, &otherEntities);
        // printf("%ld\n", allEntities.size());
        // printf("%s\n", connection.getMessageString(&allEntities).c_str());
        // printf("%ld %ld %ld\n", allEntities.size(), myEntities.size(), otherEntities.size());
        // const int totalEntities = allEntities.size();
        // for (int i = 0; i < totalEntities; i++) {

        //     Entity *entity = allEntities.at(i);
        //     // if (entity->uniqueID == -1) entity->setUniqueID(-2);
        //     // if (entity->attachedClientID == -1) entity->setAttachedClientID(-2);
        //     printf("%d %d\n", entity->attachedClientID, entity->uniqueID);

        // }

        physicsSubsystem->doPhysics(myEntities);
        collisionSubsystem->doCollision(myEntities);
        animationSubsystem->doAnimation(myEntities);

        if (connection.isClient()) {

            renderer->presentScene(allEntities);

        }

    }

    if (connection.isClient()) {

        renderer->cleanup();

    }

    return 0;
}
