#include "main.hpp"
#include <memory>
#include <vector>
#include <thread>

// Function to create entities and add them to the entity vector
void createEntities(std::vector<Entity>& E);

int main(int argc, char *argv[]) {

    bool isServer = (argc > 1 && std::string(argv[1]) == "server");

    // Global vector to hold all entities
    std::vector<Entity> E;

    // Call the function to create entities
    createEntities(E);  

    // Initialize either server or client
    Server* server = nullptr;
    Client* client = nullptr;

    // Server setup - headless mode
    if (isServer) {
        server = new Server(E);  // Initialize the server with the entity vector
        std::cout << "Server started" << std::endl;

        server->run(); // Server runs in a loop handling multiple clients
    }
    else { // Client setup
        
        // Initialize the application (SDL subsystems, renderer, etc.)
        renderer->init("Game");

        // Initialize the client
        int client_id = atoi(argv[2]);  // Pass the client_id as a command-line argument
        client = new Client(client_id, E[client_id]);
        client->performHandshake(); // Performing a handshake with the server
        std::cout << "Client " << client_id << " started, controlling entity " << client_id << "." << std::endl;

        // Main game loop for client
        int64_t last_render_time = globalTimeline->getTime();
        std::cout << last_render_time << std::endl;
        while(true) {
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;

            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            renderer->prepareScene();

            inputSubsystem->doInput(E);

            // Client-server communication
            client->sendEntityUpdate();       // Client sends its entity update to the server
            client->receiveEntityUpdates(E);  // Client receives entity updates from the server

            physicsSubsystem->doPhysics(E);
            collisionSubsystem->doCollision(E);
            animationSubsystem->doAnimation(E);

            renderer->presentScene(E);
        }

        renderer->cleanup();
    }

    return 0;
}

void createEntities(std::vector<Entity>& E) {
    // Create entities for the game
    SDL_Color shapeColor5 = {0, 0, 255, 255};  // Green color
    Entity shape5(300, 300, 100, 100, shapeColor5);
    shape5.physicsHandler = new DefaultMovementPhysicsHandler(true);
    E.push_back(shape5);

    SDL_Color shapeColor = {255, 0, 0, 255};  // Red color
    Entity shape1(1000, 0, 100, 500, shapeColor);
    shape1.physicsHandler = new DefaultGravityPhysicsHandler(false);
    E.push_back(shape1);

    // Initialize pattern-following shape
    SDL_Color shapeColor3 = {255, 255, 0, 255};  // Yellow color
    Entity shape3(10, 10, 100, 100, shapeColor3);
    std::vector<SDL_Rect> shape3Path = {
        {SCREEN_WIDTH/3, 10, 1, 1},
        {SCREEN_WIDTH/2, 10, 1, 1},
        {SCREEN_WIDTH-100, 10, 1, 1},
        {SCREEN_WIDTH/2, 10, 1, 1},
        {SCREEN_WIDTH/3, 10, 1, 1},
        {10, 10, 1, 1}
    };
    shape3.patternHandler = new DefaultPatternHandler(shape3Path);
    E.push_back(shape3);
}