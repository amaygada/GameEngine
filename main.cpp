#include "main.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <map>

void createClientCharacter(int x, int y, std::vector<Entity*>& E);
void createDeathZone(std::vector<Entity*>& E);
void createSideScroller(std::vector<Entity*>& E);
void HandleGentleExit(Client *client);

void run_client_server(bool isServer) {
    // a list of entities controlled by the process using it
    vector<Entity*> E;

    if(isServer){
        // SDL_Color shapeColor1 = {255, 0, 0, 255};  // Red color
        // Entity* shape1 = new Entity(800, 400, 50, 200, shapeColor1);
        // shape1->physicsHandler = new DefaultGravityPhysicsHandler(false);
        // E.push_back(shape1);  // Push the dynamically allocated entity

        SDL_Color shapeColor3 = {110, 110, 110, 255};
        Entity *shape3 = new Entity( 400, SCREEN_HEIGHT-200, 200, 50, shapeColor3);
        shape3->setName("Platform");
        E.push_back(shape3);

        SDL_Color shapeColor4 = {90, 90, 90, 255};
        Entity *shape4 = new Entity( 700, SCREEN_HEIGHT-400, 300, 50, shapeColor4);
        shape4->setName("Platform");
        E.push_back(shape4);

        SDL_Color shapeColor5 = {70, 70, 70, 255};
        Entity *shape5 = new Entity( 1000, SCREEN_HEIGHT-300, 400, 50, shapeColor5);
        shape5->setName("Platform");
        E.push_back(shape5);

        SDL_Color shapeColor6 = {200, 80, 80, 255};
        Entity *shape6 = new Entity( 1800, SCREEN_HEIGHT-30, 40, 40, shapeColor6);
        std::vector<SDL_Rect> shape6Path = {};
        shape6Path.push_back({100,   SCREEN_HEIGHT-30, 1, 1});
        shape6Path.push_back({1800,   SCREEN_HEIGHT-30, 1, 1});
        shape6->patternHandler = new DefaultPatternHandler(shape6Path);\
        shape6->setName("bullet");
        E.push_back(shape6);

        SDL_Color shapeColor7 = {80, 80, 10, 255};
        Entity *shape7 = new Entity( 1700, 30, 80, 80, shapeColor7);
        std::vector<SDL_Rect> shape7Path = {};
        shape7Path.push_back({1600,   100, 1, 1});
        shape7Path.push_back({1700,   200, 1, 1});
        shape7Path.push_back({1800,   100, 1, 1});
        shape7Path.push_back({1700,   30, 1, 1});
        shape7->patternHandler = new DefaultPatternHandler(shape7Path);
        shape7->setName("sun");
        E.push_back(shape7);


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
    
            physics_thread.join();
            animation_thread.join();
        }

    } else {

        vector<pair<int,int>> spawn_points;
        spawn_points.push_back({100, SCREEN_HEIGHT-70});
        spawn_points.push_back({150, SCREEN_HEIGHT-70});
        spawn_points.push_back({200, SCREEN_HEIGHT-70});
        spawn_points.push_back({250, SCREEN_HEIGHT-70});

        // select a random spawn point
        cout<< rand() % spawn_points.size()  <<endl;
        int spawn_point_index = rand() % spawn_points.size();
        int spawn_x = spawn_points[spawn_point_index].first;
        int spawn_y = spawn_points[spawn_point_index].second;

        // create client character
        createClientCharacter(spawn_x, spawn_y, E);
        createDeathZone(E);
        createSideScroller(E);

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

            HandleGentleExit(client);

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

int main(int argc, char *argv[]){
    srand(time(0));
    app->quit = false;
    bool isServer = (argc > 1 && std::string(argv[1]) == "server");
    run_client_server(isServer);
     return 0;
}

void createDeathZone(std::vector<Entity*>& E) {
    SDL_Color shapeColor2 = {0, 0, 0, 255};  // Black color
    Entity *shape2 = new Entity( 0, 0, 10, SCREEN_HEIGHT+70, shapeColor2);
    shape2->renderingHandler = new DefaultRenderer();
    shape2->setName("DeathZone");
    E.push_back(shape2);
}

void createSideScroller(std::vector<Entity*>& E) {
    SDL_Color shapeColor2 = {0, 0, 0, 255};  // Black color
    Entity *shape2 = new Entity( SCREEN_WIDTH/2, 0, 10, SCREEN_HEIGHT+70, shapeColor2);
    shape2->renderingHandler = new DefaultRenderer();
    shape2->setName("SideScroller");
    E.push_back(shape2);
}

void createClientCharacter(int x, int y, std::vector<Entity*>& E) {
    // select a random color
    uint8_t r = rand() % 256;
    uint8_t g = rand() % 256;
    uint8_t b = 255;
    SDL_Color shapeColor = {r, g, b, 255};
    Entity* shape = new Entity(x, y, 100, 100, shapeColor);
    shape->setName("Character");
    shape->renderingHandler = new DefaultRenderer();
    shape->physicsHandler2 = new JumpPhysicsHandler(true);
    shape->physicsHandler = new XPhysicsHandler(true);
    shape->collisionHandler = new CharacterCollisionHandler();
    E.push_back(shape);  // Push the dynamically allocated entity
}

void JumpPhysicsHandler::updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
    // if timer is paused, resume it
    if (this->physicsTimeline->isParentPaused()) return;

    if (this->start_time == -1) {
        this->start_time = physicsTimeline->getTime();
    }

    int timeValue = int(physicsTimeline->getTime() - this->start_time);
    
    while(true){
        timeValue = int(physicsTimeline->getTime() - this->start_time);
        int y_add = (velocity_y*timeValue) - (0.5 * acceleration_y * (timeValue * timeValue));
        if (y_add < 0) break;
        entity->y = y_initial - y_add;
    }

    entity->y = y_initial;

    physicsTimeline->pause();
    start_time = -1;
    y_initial = -1;
    
}

void JumpPhysicsHandler::handleInput(Entity *entity) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    bool parent_paused = this->physicsTimeline->isParentPaused();
    if(parent_paused) return;

    if(this->y_initial != -1) return;

    // If the 'W' key is pressed
    if (state[SDL_SCANCODE_W] && !this->PState[SDL_SCANCODE_W]) {
        this->y_initial = entity->y;
        this->physicsTimeline->resume();
        std::thread updateThread(&JumpPhysicsHandler::updatePhysics, this, entity, 0, 40, 0, 3, -1);
        updateThread.detach();
    }

    for (int i = 0; i < 512; i++) {
        this->PState[i] = state[i];
    }
}

void XPhysicsHandler::handleInput(Entity *entity) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    bool parent_paused = this->physicsTimeline->isParentPaused();
    if(parent_paused) return;

    // If the 'D' key is pressed
    if (state[SDL_SCANCODE_D] && !this->PState[SDL_SCANCODE_D]) {
        this->physicsTimeline->resume();
        this->updatePhysics(entity, 10, 0, 0, 0, 1);
    }else if (state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        this->updatePhysics(entity, 10, 0, 0, 0, 1);
    }else if (!state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        this->physicsTimeline->pause();
        this->start_time = -1;
    }

    // If the 'A' key is pressed
    if (state[SDL_SCANCODE_A] && !this->PState[SDL_SCANCODE_A]) {
        this->physicsTimeline->resume();
        this->updatePhysics(entity, 10, 0, 0, 0, -1);
    }else if (state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        this->updatePhysics(entity, 10, 0, 0, 0, -1);
    }else if (!state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        this->physicsTimeline->pause();
        this->start_time = -1;
    }

    for (int i = 0; i < 512; i++) {
        this->PState[i] = state[i];
    }
}

void XPhysicsHandler::updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
    // if timer is paused, resume it
    if (this->physicsTimeline->isParentPaused()) return;

    if (this->physicsTimeline->isPaused()) return;

    if (this->start_time == -1) {
        this->start_time = physicsTimeline->getTime();
    }

    int timeValue = int(physicsTimeline->getTime() - this->start_time);
    int to_be_loc = velocity_x + (0.5 * acceleration_x * (timeValue * timeValue));
    int locDifference = direction * (to_be_loc);

    if (entity->x + entity->w + locDifference >= SCREEN_WIDTH) {
        entity->x = SCREEN_WIDTH - entity->w;
    }else if(entity->x + locDifference <= 0){
        entity->x = 0;
    }else {
        entity->x += locDifference;
    }
}

void CharacterCollisionHandler::triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) {
    if(entity->getName() == "Character"){
        for (const auto pair : entityMap) {
            std::vector<Entity *> entities = pair.second;
            for (Entity *other : entities) {
                if(other->getName() == "DeathZone" && entity->checkCollision(*other)){
                    std::vector<std::pair<int, int>> spawn_points;
                    spawn_points.push_back({100, SCREEN_HEIGHT-70});
                    spawn_points.push_back({150, SCREEN_HEIGHT-70});
                    spawn_points.push_back({200, SCREEN_HEIGHT-70});
                    spawn_points.push_back({250, SCREEN_HEIGHT-70});

                    // select a random spawn point
                    cout<< rand() % spawn_points.size()  <<endl;
                    int spawn_point_index = rand() % spawn_points.size();
                    int spawn_x = spawn_points[spawn_point_index].first;
                    int spawn_y = spawn_points[spawn_point_index].second;

                    entity->x = spawn_x;
                    entity->y = spawn_y;
                }else if(other->getName() == "bullet" && entity->checkCollision(*other)){
                    cout<<"Character hit by bullet"<<endl;
                }else if(other->getName() == "SideScroller" && entity->checkCollision(*other)){
                    cout<<"Scroll to side now"<<endl;
                }
            }
        }
    }
}

void HandleGentleExit(Client *client){
    if(app->quit){
        client->sendQuitMessage();
        SDL_Quit();
        exit(0);
    }
}