#include "main.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <map>

void createClientEntities(std::vector<Entity*>& E);

int main(int argc, char *argv[]){
    bool isServer = (argc > 1 && std::string(argv[1]) == "server");

    // a list of entities controlled by the process using it
    vector<Entity*> E;

    if(isServer){
        SDL_Color shapeColor1 = {255, 0, 0, 255};  // Red color
        Entity* shape1 = new Entity(800, 400, 50, 200, shapeColor1);
        shape1->physicsHandler = new DefaultGravityPhysicsHandler(false);
        E.push_back(shape1);  // Push the dynamically allocated entity

        SDL_Color shapeColor3 = {255, 255, 0, 255};  // Yellow color
        Entity shape3( 10, 10, 100, 100, shapeColor3);
        std::vector<SDL_Rect> shape3Path = {};
        shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH-100,10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
        shape3Path.push_back({10, 10, 1, 1});
        shape3.patternHandler = new DefaultPatternHandler(shape3Path);
        E.push_back(&shape3);
        
        Server *server = new Server();
        server->addEntities(E);
        server->run();

        int64_t last_render_time = globalTimeline->getTime();
        while(1){
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;
            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            std::thread physics_thread(&PhysicsSubsystem::doPhysics, physicsSubsystem, std::ref(server->entityMap[-1]));
            std::thread animation_thread(&AnimationSubsystem::doAnimation, animationSubsystem, std::ref(server->entityMap[-1]));
            std::thread collision_thread(&CollisionSubsystem::doCollision, collisionSubsystem, std::ref(server->entityMap));
    
            physics_thread.join();
            animation_thread.join();
            collision_thread.join();
        }

    } else {
        // create entities
        createClientEntities(E);

        Client *client = new Client();
        
        // perform handshake
        client->performHandshake(E);

        renderer->init("Game");

        // Main game loop for client
        int64_t last_render_time = globalTimeline->getTime();
        while(true) {
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;

            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            renderer->prepareScene();

            std::thread input_thread(&InputSubsystem::doInput, inputSubsystem, std::ref(client->getEntityMap()[client->id]));

            std::thread physics_thread(&PhysicsSubsystem::doPhysics, physicsSubsystem, std::ref(client->getEntityMap()[client->id]));

            // inputSubsystem->doInput(client->getEntityMap()[client->id]);

            // Client-server communication
            client->sendEntityUpdate();       // Client sends its entity update to the server
            client->receiveEntityUpdates();  // Client receives entity updates from the server

            // physicsSubsystem->doPhysics(client->getEntityMap()[client->id]);

            renderer->presentScene(client->getEntityMap());

            input_thread.join();
            physics_thread.join();
        }

        renderer->cleanup();
    }

    return 0;
}

void createClientEntities(std::vector<Entity*>& E) {

    // Create entities for the game
    SDL_Color shapeColor1 = {0, 0, 255, 255};  // Blue color
    Entity* shape1 = new Entity(300, 300, 100, 100, shapeColor1);
    shape1->physicsHandler = new DefaultMovementPhysicsHandler(true);
    E.push_back(shape1);  // Push the dynamically allocated entity

    // Create entities for the game
    SDL_Color shapeColor2 = {255, 0, 255, 255};  // Pink color
    Entity* shape2 = new Entity(500, 500, 150, 150, shapeColor2);
    shape2->physicsHandler = new DefaultMovementPhysicsHandler(true);
    E.push_back(shape2);  // Push the dynamically allocated entity

    // Initialize pattern-following shape
    SDL_Color shapeColor3 = {255, 0, 0, 255};  // Red color
    Entity* shape3 = new Entity(700, 700, 150, 150, shapeColor3);
    shape3->physicsHandler = new DefaultMovementPhysicsHandler(true);
    E.push_back(shape3);  // Push the dynamically allocated entity
}