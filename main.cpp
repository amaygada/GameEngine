#include "main.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <map>
#include <random>

void createClientEntities(std::vector<Entity*>& E);
void createPlatformEntities(std::vector<Entity*>& E);

void run_client_server(bool isServer) {
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

        vector<Entity *> spawnPoints = {};
        SDL_Color spColor = {0, 0, 0, 0}; // Invisible

        // First spawn point. With the current implementation entities will be defaulted to this spawn point upon initial connection. Any additional functionality regarding spawn points is to be left to individual game implementations
        Entity *sp1 = new Entity(300, 300, 0, 0, spColor);
        spawnPoints.push_back(sp1);

        // Second spawn point
        Entity *sp2 = new Entity(600, 300, 0, 0, spColor);
        spawnPoints.push_back(sp2);
        
        Server *server = new Server();
        server->addEntities(E, spawnPoints);
        server->run();

        int64_t last_render_time = globalTimeline->getTime();
        while(1){
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;
            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            std::thread physics_thread(&PhysicsSubsystem::doPhysics, physicsSubsystem, std::ref(server->entityMap[-1]));
            std::thread animation_thread(&AnimationSubsystem::doAnimation, animationSubsystem, std::ref(server->entityMap[-1]));
            // std::thread collision_thread(&CollisionSubsystem::doCollision, collisionSubsystem, std::ref(server->entityMap));
    
            physics_thread.join();
            animation_thread.join();
            // collision_thread.join();
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

            // collisionSubsystem->doCollision(client->getEntityMap()[client->id], client->getEntityMap());
            std::thread collision_thread(&CollisionSubsystem::doCollision, collisionSubsystem, std::ref(client->getEntityMap()[client->id]), std::ref(client->getEntityMap()));
            std::thread input_thread(&InputSubsystem::doInput, inputSubsystem, std::ref(client->getEntityMap()[client->id]));
            std::thread physics_thread(&PhysicsSubsystem::doPhysics, physicsSubsystem, std::ref(client->getEntityMap()[client->id]));
            
            input_thread.join();
            physics_thread.join();
            collision_thread.join();

            // Client-server communication
            client->sendEntityUpdate();       // Client sends its entity update to the server
            client->receiveEntityUpdates();  // Client receives entity updates from the server
            
            renderer->presentScene(client->getEntityMap(), client->id);
        }

        renderer->cleanup();
    }
}

void run_p2p() {

    // P2PClient *client = new P2PClient();

    // vector<Entity*> E;

    // client->expose_port_and_connect();

    // client->request_id();

    // client->announce_new_peer();

    // // create entities
    // createClientEntities(E);
    
    // // platform entity
    // SDL_Color shapeColor3 = {255, 255, 0, 255};  // Yellow color
    // Entity shape3( 10, 10, 100, 100, shapeColor3);
    // std::vector<SDL_Rect> shape3Path = {};
    // shape3Path.push_back({SCREEN_WIDTH-100,10, 1, 1});
    // shape3Path.push_back({10, 10, 1, 1});
    // shape3.patternHandler = new DefaultPatternHandler(shape3Path);

    // vector<Entity*> temp;
    // temp.push_back(E[client->get_id()]);
    // client->entityMap[client->get_id()] = temp;
    // client->entityMap[-1].push_back(&shape3);

    // renderer->init("Game");

    // // Main game loop for client
    // int64_t last_render_time = globalTimeline->getTime();
    // while(true) {
    //     int64_t frame_delta = globalTimeline->getTime() - last_render_time;

    //     if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
    //     last_render_time = globalTimeline->getTime();

    //     renderer->prepareScene();

    //     inputSubsystem->doInput(client->entityMap[client->get_id()]);
    //     physicsSubsystem->doPhysics(client->entityMap[client->get_id()]);
    //     animationSubsystem->doAnimation(client->entityMap[-1]);
    //     collisionSubsystem->doCollision(client->entityMap);

    //     client->publish_entity_positions();       // Client sends its entity update to the server
    //     client->receive_entity_positions();  // Client receives entity updates from the server

    //     renderer->presentScene(client->get_entity_map());
    // }

    // renderer->cleanup();
}

int main(int argc, char *argv[]){
    if(argc > 1 && std::string(argv[1]) == "pp") {
        run_p2p();
    }else{
        bool isServer = (argc > 1 && std::string(argv[1]) == "server");
        run_client_server(isServer);
    }
    return 0;
}

SDL_Color getRandColor() {

    const int min = 0;
    const int max = 255;
    std::random_device rand;
    std::mt19937 gen(rand());
    std::uniform_int_distribution<> distr(min, max);

    Uint8 r = distr(gen), g = distr(gen), b = distr(gen), a = 255;
    while (r == BACKGROUND_COLOR_R && g == BACKGROUND_COLOR_G && b == BACKGROUND_COLOR_B) {

        r = distr(gen);
        g = distr(gen);
        b = distr(gen);

    }

    return {r, g, b, a};

}

void createClientEntities(std::vector<Entity*>& E) {

    // Create client entity for the game
    SDL_Color shapeColor1 = getRandColor();
    Entity *shape1 = new Entity(-999, -999, 100, 100, shapeColor1);
    shape1->physicsHandler = new DefaultMovementPhysicsHandler(true);
    shape1->collisionHandler = new DefaultCollisionHandler();
    shape1->renderingHandler = new DefaultRenderer();
    E.push_back(shape1);

}

void createPlatformEntities(std::vector<Entity*>& PE) {
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
    PE.push_back(&shape3);
}