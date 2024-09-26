#include "main.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <map>

// Function to create entities and add them to the entity vector
void createEntities(std::vector<Entity*>& E);

int main(int argc, char *argv[]) {

    bool isServer = (argc > 1 && std::string(argv[1]) == "server");

    // Global vector to hold all entities
    vector<Entity*> E;

    // Initialize either server or client
    Server* server = nullptr;

    // Server setup - headless mode
    if (isServer) {
        server = new Server();  // Initialize the server with the entity vector
        std::cout << "Server started" << std::endl;

        server->run(); // Server runs in a loop handling multiple clients
    }
    else { // Client setup
        
        // Initialize the application (SDL subsystems, renderer, etc.)
        renderer->init("Game");

        // Call the function to create entities
        createEntities(E);  

        // Initialize the client
        Client* client = nullptr;
        client = new Client(E[1]);
        client->performHandshake(); // Performing a handshake with the server
        //std::unordered_map<int, Entity*> entity_map = client->getEntityMap();

        // Main game loop for client
        int64_t last_render_time = globalTimeline->getTime();
        std::cout << last_render_time << std::endl;
        while(true) {
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;

            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            renderer->prepareScene();

            inputSubsystem->doInput(client->getEntityMap());

            // Client-server communication
            client->sendEntityUpdate();       // Client sends its entity update to the server
            client->receiveEntityUpdates();  // Client receives entity updates from the server

            physicsSubsystem->doPhysics(client->getEntityMap());
            collisionSubsystem->doCollision(client->getEntityMap());
            //animationSubsystem->doAnimation(E);

            renderer->presentScene(client->getEntityMap());
        }

        renderer->cleanup();
    }

    return 0;
}

void createEntities(std::vector<Entity*>& E) {
    // Create entities for the game
    SDL_Color shapeColor1 = {0, 0, 255, 255};  // Blue color
    Entity* shape1 = new Entity(300, 300, 100, 100, shapeColor1);
    shape1->physicsHandler = new DefaultMovementPhysicsHandler(true);
    E.push_back(shape1);  // Push the dynamically allocated entity

    // Create entities for the game
    SDL_Color shapeColor2 = {255, 0, 255, 255};  // Red color
    Entity* shape2 = new Entity(500, 500, 150, 150, shapeColor2);
    shape2->physicsHandler = new DefaultMovementPhysicsHandler(true);
    E.push_back(shape2);  // Push the dynamically allocated entity



    // Initialize pattern-following shape
    SDL_Color shapeColor3 = {255, 255, 0, 255};  // Red color
    Entity* shape3 = new Entity(700, 700, 150, 150, shapeColor3);
    //shape2->physicsHandler = new DefaultMovementPhysicsHandler(true);
    //shape3.patternHandler = new DefaultPatternHandler(shape3Path);
    E.push_back(shape3);  // Push the dynamically allocated entity
}