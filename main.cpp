#include "main.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <map>

void createClientCharacter(int x, int y, std::vector<Entity*>& E);
void createDeathZone(std::vector<Entity*>& E);
void createSideScroller(std::vector<Entity*>& E);
void HandleGentleExit(Client *client);
void custom_entity_renderer(unordered_map<int, std::vector<Entity *>> &entity_map, int client_id);
void sendCustomEntityUpdate(Client *client);
void create_events();

std::map<std::string, Event*> eventMap;
vector<Entity*> E;
bool moving = false;

Server *server;
Client *client;

void run_client_server(bool isServer) {
    // a list of entities controlled by the process using it

    if(isServer){
        server = new Server();
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

        SDL_Color shapeColor6 = {200, 80, 80, 255};
        Entity *shape6 = new Entity( 1900, SCREEN_HEIGHT-30, 40, 40, shapeColor6);
        std::vector<SDL_Rect> shape6Path = {};
        shape6Path.push_back({100,   SCREEN_HEIGHT-30, 1, 1});
        shape6Path.push_back({1800,   SCREEN_HEIGHT-30, 1, 1});
        shape6->patternHandler = new BulletMovementHandler(server);
        shape6->setName("bullet");
        E.push_back(shape6);

        eventManager->registerEvent("PlayerExitEvent", new PlayerExitEventHandler());

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

            while (!eventManager->eventQueue.empty() && eventManager->eventQueue.top().first <= eventManager->timeline->getTime()) {
                // cout<<eventManager->eventQueue.top().second->type<<endl;
                std::pair<int, Event*> eventPair = eventManager->eventQueue.top();
                eventManager->eventQueue.pop();
                Event *event = eventPair.second;
                std::string eventType = event->type;
                std::vector<EventHandler*> handlers = eventManager->eventHandlers[eventType];
                for (EventHandler *handler : handlers) {
                    handler->onEvent(*event);
                }
            }
        }

    } else {
        client = new Client();

        // create a spawn event
        Event *spawnEvent = eventMap["SpawnEvent"];
        spawnEvent->addParameter("init", 1);
        eventManager->raiseEvent(spawnEvent, 0);

        createDeathZone(E);
        createSideScroller(E);

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

            while (!eventManager->eventQueue.empty() && eventManager->eventQueue.top().first <= eventManager->timeline->getTime()) {
                std::pair<int, Event*> eventPair = eventManager->eventQueue.top();
                eventManager->eventQueue.pop();
                Event *event = eventPair.second;
                std::string eventType = event->type;
                std::vector<EventHandler*> handlers = eventManager->eventHandlers[eventType];
                for (EventHandler *handler : handlers) {
                    handler->onEvent(*event);
                }
            }

            client->performHandshake(E); //just once

            // Client-server communication
            // client->sendEntityUpdate();       // Client sends its entity update to the server
            sendCustomEntityUpdate(client);
            client->receiveEntityUpdates();  // Client receives entity updates from the server
            
            if(app->replay == false){
                custom_entity_renderer(client->getEntityMap(), client->id);
            }
            renderer->presentScene(client->getEntityMap(), client->id, true);
        }

        renderer->cleanup();
    }
}

int main(int argc, char *argv[]){
    srand(time(0));
    app->quit = false;
    app->displacement = 0;

    create_events();

    bool isServer = (argc > 1 && std::string(argv[1]) == "server");
    run_client_server(isServer);
     return 0;
}

void BulletMovementHandler::moveToPath(Entity *entity, int factor) {
    if (this->patternHandlerTimeline->isParentPaused()) return;

    int64_t currentTime = patternHandlerTimeline->getTime();

    if (this->start_time == -1) {
        this->start_time = currentTime;
    }

    int timeDiff = int(currentTime - this->start_time);
    this->start_time = currentTime;

    entity->x = entity->x - factor;

    if(entity->x < 300){
        // kill the entity
        // create an event to create a new entity that is added to the server list.
        E.pop_back();
        delete entity;
        eventManager->raiseEvent(eventMap["BulletEvent"], 2);
    }
    
}

void BulletEventHandler::onEvent(Event e) {
    if(e.type == "BulletEvent"){
        // Server *server = e.getParameter("server")->m_asServerObject;
        SDL_Color shapeColor6 = {200, 80, 80, 255};
        Entity *shape6 = new Entity( 1900, SCREEN_HEIGHT-30, 40, 40, shapeColor6);
        std::vector<SDL_Rect> shape6Path = {};
        shape6Path.push_back({100,   SCREEN_HEIGHT-30, 1, 1});
        shape6Path.push_back({1800,   SCREEN_HEIGHT-30, 1, 1});
        shape6->patternHandler = new BulletMovementHandler(server);
        shape6->setName("bullet");
        E.push_back(shape6);
    }
}

void SpawnEventHandler::onEvent(Event e) {
    if(e.type == "SpawnEvent"){
        vector<pair<int,int>> spawn_points;
        spawn_points.push_back({e.getParameter("x1")->m_asInt, e.getParameter("y1")->m_asInt});
        spawn_points.push_back({e.getParameter("x2")->m_asInt, e.getParameter("y2")->m_asInt});
        spawn_points.push_back({e.getParameter("x3")->m_asInt, e.getParameter("y3")->m_asInt});
        spawn_points.push_back({e.getParameter("x4")->m_asInt, e.getParameter("y4")->m_asInt});

        // select a random spawn point
        cout<< rand() % spawn_points.size()  <<endl;
        int spawn_point_index = rand() % spawn_points.size();
        int spawn_x = spawn_points[spawn_point_index].first;
        int spawn_y = spawn_points[spawn_point_index].second;

        if(e.getParameter("init")->m_asInt == 1){
            createClientCharacter(spawn_x, spawn_y, E);
        }else{
            e.getParameter("Entity")->m_asGameObject->x = spawn_x;
            e.getParameter("Entity")->m_asGameObject->y = spawn_y;
            app->displacement = 0;
        }
    }
}

void SideScrollingEventHandler::onEvent(Event e) {
    if(e.type == "SideScrollingEvent"){
        if(moving){
            if(e.getParameter("Entity")->m_asGameObject->x >= SCREEN_WIDTH/2 + 1 - 100){
                app->displacement += 15;
            }
        }
    }
}

void create_events(){
    Event *spawnEvent = new Event("SpawnEvent");
    spawnEvent->addParameter("x1", 100);
    spawnEvent->addParameter("y1", SCREEN_HEIGHT-70);
    spawnEvent->addParameter("x2", 150);
    spawnEvent->addParameter("y2", SCREEN_HEIGHT-70);
    spawnEvent->addParameter("x3", 200);
    spawnEvent->addParameter("y3", SCREEN_HEIGHT-70);
    spawnEvent->addParameter("x4", 250);
    spawnEvent->addParameter("y4", SCREEN_HEIGHT-70);
    eventMap["SpawnEvent"] = spawnEvent;

    Event *goRightEvent = new Event("GoRightEvent");
    goRightEvent->addParameter("velocityX", double(10));
    goRightEvent->addParameter("velocityY", double(0));
    goRightEvent->addParameter("accelerationX", double(0));
    goRightEvent->addParameter("accelerationY", double(0));
    goRightEvent->addParameter("direction", 1);
    eventMap["GoRightEvent"] = goRightEvent;

    Event *goLeftEvent = new Event("GoLeftEvent");
    goLeftEvent->addParameter("velocityX", double(10));
    goLeftEvent->addParameter("velocityY", double(0));
    goLeftEvent->addParameter("accelerationX", double(0));
    goLeftEvent->addParameter("accelerationY", double(0));
    goLeftEvent->addParameter("direction", -1);
    eventMap["GoLeftEvent"] = goLeftEvent;

    Event *jumpEvent = new Event("JumpEvent");
    jumpEvent->addParameter("velocityX", double(0));
    jumpEvent->addParameter("velocityY", double(40));
    jumpEvent->addParameter("accelerationX", double(0));
    jumpEvent->addParameter("accelerationY", double(3));
    jumpEvent->addParameter("direction", -1);
    eventMap["JumpEvent"] = jumpEvent;

    Event *dashEvent = new Event("DashEvent");
    dashEvent->addParameter("velocityX", 80.0);         // Dash speed
    dashEvent->addParameter("velocityY", 0.0);
    dashEvent->addParameter("accelerationX", 3.0);      // Acceleration for dash
    dashEvent->addParameter("accelerationY", 0.0);
    dashEvent->addParameter("direction", 1);            // Right direction
    eventMap["DashEvent"] = dashEvent;

    Event *bulletEvent = new Event("BulletEvent");
    eventMap["BulletEvent"] = bulletEvent;

    eventManager->registerEvent("BulletEvent", new BulletEventHandler());
    eventManager->registerEvent("SpawnEvent", new SpawnEventHandler());
    eventManager->registerEvent("SideScrollingEvent", new SideScrollingEventHandler());
    eventManager->registerEvent("GoRightEvent", new GoRightEventHandler());
    eventManager->registerEvent("GoLeftEvent", new GoLeftEventHandler());

    eventManager->registerEvent("DashEvent", new DashEventHandler());

}

void createDeathZone(std::vector<Entity*>& E) {
    SDL_Color shapeColor2 = {0, 0, 0, 255};  // Black color
    Entity *shape2 = new Entity( 0, 0, 10, SCREEN_HEIGHT+70, shapeColor2);
    // shape2->renderingHandler = new DefaultRenderer();
    shape2->setName("DeathZone");
    E.push_back(shape2);
}

void createSideScroller(std::vector<Entity*>& E) {
    SDL_Color shapeColor2 = {0, 0, 0, 255};  // Black color
    Entity *shape2 = new Entity( SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT+70, shapeColor2);
    // shape2->renderingHandler = new DefaultRenderer();
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

void updatePhysicsX(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
    // if timer is paused, resume it
    if (entity->physicsHandler->physicsTimeline->isParentPaused()) return;

    if (entity->physicsHandler->physicsTimeline->isPaused()) return;

    if (entity->physicsHandler->start_time == -1) {
        entity->physicsHandler->start_time = entity->physicsHandler->physicsTimeline->getTime();
    }

    int timeValue = int(entity->physicsHandler->physicsTimeline->getTime() - entity->physicsHandler->start_time);
    int to_be_loc = velocity_x + (0.5 * acceleration_x * (timeValue * timeValue));
    int locDifference = direction * (to_be_loc);

    int xbound = SCREEN_WIDTH/1.5;

    if (entity->x + entity->w + locDifference >= xbound) {
        entity->x = xbound - entity->w;
    }else if(entity->x + locDifference <= 0){
        entity->x = 0;
    }else {
        entity->x += locDifference;
    }
}

void PlayerExitEventHandler::onEvent(Event e) {
    if(e.type == "PlayerExitEvent"){
        int client_id = e.getParameter("Client Id")->m_asInt;
        // cout<<"Player "<<client_id<<" exited"<<endl;
        auto it = server->entityMap.find(client_id);
        server->entityMap.erase(it);
    }
}

void GoRightEventHandler::onEvent(Event e) {
    if(e.type == "GoRightEvent"){
        Entity *entity = e.getParameter("Entity")->m_asGameObject;
        double velocity_x = e.getParameter("velocityX")->m_asDouble;
        double velocity_y = e.getParameter("velocityY")->m_asDouble;
        double acceleration_x = e.getParameter("accelerationX")->m_asDouble;
        double acceleration_y = e.getParameter("accelerationY")->m_asDouble;
        int direction = e.getParameter("direction")->m_asInt;
        updatePhysicsX(entity, velocity_x, velocity_y, acceleration_x, acceleration_y, direction);
    }
}

void GoLeftEventHandler::onEvent(Event e) {
    if(e.type == "GoLeftEvent"){
        Entity *entity = e.getParameter("Entity")->m_asGameObject;
        double velocity_x = e.getParameter("velocityX")->m_asDouble;
        double velocity_y = e.getParameter("velocityY")->m_asDouble;
        double acceleration_x = e.getParameter("accelerationX")->m_asDouble;
        double acceleration_y = e.getParameter("accelerationY")->m_asDouble;
        int direction = e.getParameter("direction")->m_asInt;
        updatePhysicsX(entity, velocity_x, velocity_y, acceleration_x, acceleration_y, direction);
    }
}

void DashEventHandler::onEvent(Event e) {
    if (e.type == "DashEvent") {
        Entity *entity = e.getParameter("Entity")->m_asGameObject;
        double velocity_x = e.getParameter("velocityX")->m_asDouble;
        double velocity_y = e.getParameter("velocityY")->m_asDouble;
        double acceleration_x = e.getParameter("accelerationX")->m_asDouble;
        double acceleration_y = e.getParameter("accelerationY")->m_asDouble;
        int direction = e.getParameter("direction")->m_asInt;
        updatePhysicsX(entity, velocity_x, velocity_y, acceleration_x, acceleration_y, direction);
    }   
}

void XPhysicsHandler::handleInput(Entity *entity) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    int currentTime = SDL_GetTicks();  // Get the current time in milliseconds

    bool parent_paused = this->physicsTimeline->isParentPaused();
    if (parent_paused) return;

    // Track "D" press state and timestamp
    if (state[SDL_SCANCODE_D]) {
        lastDPressTime = currentTime;
        isDPressed = true;
    } else {
        isDPressed = false;
    }

    // Track "Shift" press state and timestamp
    if (state[SDL_SCANCODE_LSHIFT]) {
        lastShiftPressTime = currentTime;
        isShiftPressed = true;
    } else {
        isShiftPressed = false;
    }

    // Check if both "Shift" and "D" are pressed within the buffer time
    if (state[SDL_SCANCODE_D] && state[SDL_SCANCODE_LSHIFT]) {

        // Update last press times
        if (!this->PState[SDL_SCANCODE_D]) lastDPressTime = currentTime;
        if (!this->PState[SDL_SCANCODE_LSHIFT]) lastShiftPressTime = currentTime;

        // If "Shift + D" pressed within the dash buffer time
        if (abs(lastDPressTime - lastShiftPressTime) <= dashBufferTime) {
            // Trigger dash event if this is the first detection of "Shift + D" in this press cycle
            if (!(this->PState[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_LSHIFT])) {
                this->physicsTimeline->resume();
                Event *dashEvent = eventMap["DashEvent"];
                dashEvent->addParameter("Entity", entity);
                eventManager->raiseEvent(dashEvent, 0);
                moving = true;
            }
        }
    } else if ((this->PState[SDL_SCANCODE_D] && !state[SDL_SCANCODE_D]) ||
        (this->PState[SDL_SCANCODE_LSHIFT] && !state[SDL_SCANCODE_LSHIFT])) {
        this->physicsTimeline->pause();
        this->start_time = -1;
        moving = false;
    }

    // If the 'D' key is pressed
    if (state[SDL_SCANCODE_D] && !this->PState[SDL_SCANCODE_D]) {
        this->physicsTimeline->resume();
        Event *goRightEvent = eventMap["GoRightEvent"];
        goRightEvent->addParameter("Entity", entity);
        eventManager->raiseEvent(goRightEvent, 0);
        moving = true;
    }else if (state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        Event *goRightEvent = eventMap["GoRightEvent"];
        goRightEvent->addParameter("Entity", entity);
        eventManager->raiseEvent(goRightEvent, 0);
        moving = true;
    }else if (!state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        this->physicsTimeline->pause();
        this->start_time = -1;
        moving = false;
    }

    // If the 'A' key is pressed
    if (state[SDL_SCANCODE_A] && !this->PState[SDL_SCANCODE_A]) {
        this->physicsTimeline->resume();
        Event *goLeftEvent = eventMap["GoLeftEvent"];
        goLeftEvent->addParameter("Entity", entity);
        eventManager->raiseEvent(goLeftEvent, 0);
    }else if (state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        Event *goLeftEvent = eventMap["GoLeftEvent"];
        goLeftEvent->addParameter("Entity", entity);
        eventManager->raiseEvent(goLeftEvent, 0);
    }else if (!state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        this->physicsTimeline->pause();
        this->start_time = -1;
    }

    for (int i = 0; i < 512; i++) {
        this->PState[i] = state[i];
    }
}

void XPhysicsHandler::updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
    if (this->physicsTimeline->isParentPaused()) return;

    if (this->physicsTimeline->isPaused()) return;

    if (this->start_time == -1) {
        this->start_time = physicsTimeline->getTime();
    }

    int timeValue = int(physicsTimeline->getTime() - this->start_time);
    int to_be_loc = velocity_x + (0.5 * acceleration_x * (timeValue * timeValue));
    int locDifference = direction * (to_be_loc);

    int xbound = SCREEN_WIDTH/1.5;

    if (entity->x + entity->w + locDifference >= xbound) {
        entity->x = xbound - entity->w;
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
                    Event *spawnEvent = eventMap["SpawnEvent"];
                    spawnEvent->addParameter("Entity", entity);
                    spawnEvent->addParameter("init", 0);
                    eventManager->raiseEvent(spawnEvent, 0);
                }else if(other->getName() == "bullet" && entity->checkCollision(*other)){
                    // cout<<"Character hit by bullet"<<endl;
                }else if(other->getName() == "SideScroller" && entity->checkCollision(*other)){
                    if(!moving) continue;
                    Event *sideScrollingEvent = new Event("SideScrollingEvent");
                    sideScrollingEvent->addParameter("Entity", entity);
                    eventManager->raiseEvent(sideScrollingEvent, 0);
                }
            }
        }
    }
}

void HandleGentleExit(Client *client){
    if(app->quit){

        Event *sendServerEvent = new Event("PlayerExitEvent");
        sendServerEvent->addParameter("Client Id", client->id);

        string msg = client->serializer.serializeEvent(sendServerEvent, eventManager->timeline->getTime()+2);
        string message = client->messageHandler.createMessage(0, msg);
        client->messageHandler.sendMessage(client->push_pull, message);
        
        // client->uponTermination();
        SDL_Quit();
        exit(0);
    }
}

void custom_entity_renderer(unordered_map<int, std::vector<Entity *>> &entity_map, int client_id) {
    vector<pair<Entity,Entity>> platforms;
    std::vector<Entity *> server_entities;

    for (const auto pair : entity_map) {
        std::vector<Entity *> entities = pair.second;
        if(pair.first == -1){
            for(Entity *e: entities){
                if(e->name != "Platform") continue;
                Entity *e1 = new Entity( e->x, e->y, e->w, e->h, e->color);
                Entity *e2 = new Entity( e->x + SCREEN_WIDTH , e->y, e->w, e->h, e->color);
                e1->name = "Platform";
                e2->name = "Platform";
                platforms.push_back({*e1, *e2});
                // server_entities.push_back(e1);
                // server_entities.push_back(e2);
            }
        }

        // update_platforms();
        for (Entity *entity : entities) {
            if(pair.first == client_id){
                if(entity->renderingHandler != nullptr) entity->renderingHandler->renderEntity(entity);
            }else if(pair.first == -1){
                if(entity->name != "Platform") {
                    entity->draw(app->renderer);
                    server_entities.push_back(entity);
                }else{
                    // render platforms
                    for(auto platform: platforms){
                        platform.first.x -= app->displacement%SCREEN_WIDTH;
                        platform.second.x -= app->displacement%SCREEN_WIDTH;
                        platform.first.draw(app->renderer);
                        platform.second.draw(app->renderer);
                    }
                }
            }else if(entity->getName() == "Character"){
                if(entity->x >= app->displacement && entity->x < app->displacement + SCREEN_WIDTH){
                    entity->x -= app->displacement;
                    entity->draw(app->renderer);
                }
            }
            else entity->draw(app->renderer);
        }
    }

    // add platforms to the server entities
    for(auto platform: platforms){
        // create a copy of entity and add it to the server entities

        Entity *e1 = new Entity( platform.first.x - app->displacement%SCREEN_WIDTH, platform.first.y, platform.first.w, platform.first.h, platform.first.color);
        Entity *e2 = new Entity( platform.second.x - app->displacement%SCREEN_WIDTH, platform.second.y, platform.second.w, platform.second.h, platform.second.color);
        e1->name = "Platform";
        e2->name = "Platform";

        server_entities.push_back(e1);
        server_entities.push_back(e2);
    }    

    // cout<<server_entities.size()<<endl;

    entity_map[-1] = server_entities;
    client->setEntityMap(entity_map);

}

void sendCustomEntityUpdate(Client *client){
    Entity *entity_disp = new Entity(client->entity->x + app->displacement, client->entity->y, client->entity->w, client->entity->h, client->entity->color);
    entity_disp->name = "Character";
    string entity_str = client->serializer.serializeEntity(entity_disp);
    string id_str = std::to_string(client->id);
    string message = client->messageHandler.createMessage(3, "ClientID:"+id_str+" Entity:"+entity_str);
    client->messageHandler.sendMessage(client->push_pull, message);
}