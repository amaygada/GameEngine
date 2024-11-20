#include "main.hpp"

std::map<std::string, Event*> eventMap;
vector<Entity*> E;
bool moving = false;

int character_bullet_id = 0;
int enemy_bullet_id = 0;

CustomServer *server;
Client *client;

void create_events();
void createDeathZone(std::vector<Entity*>& E);
void createClientCharacter(int x, int y, std::vector<Entity*>& E);
void createEnemyCharacter(int x, int y, std::vector<Entity*>& E, int id);
void createBulletEnemy(int x, int y, std::vector<Entity*>& E, int id);
void createBulletCharacter(int x, int y, std::vector<Entity*>& E);
void HandleGentleExit(Client *client);
void checkGameOver();
vector<string> split(string s, char delimiter);

int health = 100;
int score = 0;

Text *healthText = nullptr;
Text *scoreText = nullptr;
Text *gameOverText = nullptr;

void run_client_server(bool isServer) {
    if(isServer){
        server = new CustomServer();

        createEnemyCharacter(100, 10, E, 0);
        createEnemyCharacter(500, 10, E, 1);
        // createEnemyCharacter(900, 10, E, 2);
        // createEnemyCharacter(1300, 10, E, 3);
        // createEnemyCharacter(1700, 10, E, 4);

        // createEnemyCharacter(300, 100, E, 5);
        // createEnemyCharacter(700, 100, E, 6);
        // createEnemyCharacter(1100, 100, E, 7);
        // createEnemyCharacter(1500, 100, E, 8);
        // createEnemyCharacter(1900, 100, E, 9);
        
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
                if(gameTimeline->isPaused()) continue;
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

        createDeathZone(E);
        createDeathZone(E);
        createClientCharacter(100, SCREEN_HEIGHT-70, E);

        renderer->init("Game");

        // Main game loop for client
        int64_t last_render_time = globalTimeline->getTime();
        while(true) {
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;

            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            healthText->setText("Health: " + std::to_string(health));
            scoreText->setText("Score: " + std::to_string(score));

            renderer->prepareScene();
            healthText->renderText();
            scoreText->renderText();
            gameOverText->renderText();
            HandleGentleExit(client);

            std::thread collision_thread(&CollisionSubsystem::doCollision, collisionSubsystem, std::ref(client->getEntityMap()[client->id]), std::ref(client->getEntityMap()));
            std::thread input_thread(&InputSubsystem::doInput, inputSubsystem, std::ref(client->getEntityMap()[client->id]));
            std::thread physics_thread(&PhysicsSubsystem::doPhysics, physicsSubsystem, std::ref(client->getEntityMap()[client->id]));
            std::thread animation_thread(&AnimationSubsystem::doAnimation, animationSubsystem, std::ref(client->getEntityMap()[client->id]));
            
            input_thread.join();
            physics_thread.join();
            collision_thread.join();
            animation_thread.join();

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
            client->sendEntityUpdate();       // Client sends its entity update to the server
            client->receiveEntityUpdates();  // Client receives entity updates from the server
            
            renderer->presentScene(client->getEntityMap(), client->id, false);
            checkGameOver();
            
        }

        renderer->cleanup();
    }
}

int main(int argc, char *argv[]){
    srand(time(0));
    app->quit = false;
    app->displacement = 0;

    create_events();

    healthText = new Text("Health: 100", 100, 110, 120, 50, {0, 0, 0, 255});
    scoreText = new Text("Score: 0", 100, 150, 120, 50, {0, 0, 0, 255});
    gameOverText = new Text("", 500, 500, 240, 100, {0, 0, 0, 255});

    bool isServer = (argc > 1 && std::string(argv[1]) == "server");
    run_client_server(isServer);
     return 0;
}

//////////////////////////////////////// ENTITIES ////////////////////////////////////////
void createDeathZone(std::vector<Entity*>& E) {
    SDL_Color shapeColor2 = {0, 0, 0, 255};  // Black color
    Entity *shape2 = new Entity( 0, 0, 10, SCREEN_HEIGHT+70, shapeColor2);
    // shape2->renderingHandler = new DefaultRenderer();
    shape2->setName("DeathZone");
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
    shape->physicsHandler = new XPhysicsHandler(true);
    shape->collisionHandler = new CharacterCollisionHandler();
    E.push_back(shape);  // Push the dynamically allocated entity
}

void createBulletCharacter(int x, int y, std::vector<Entity*>& E) {
    SDL_Color shapeColor = {80, 80, 200, 255};
    Entity *shape = new Entity( x, y, 10, 20, shapeColor);
    std::vector<SDL_Rect> shapePath = {};
    shape->patternHandler = new BulletMovementHandler();
    shape->renderingHandler = new DefaultRenderer();
    shape->collisionHandler = new CharacterBulletCollisionHandler();
    character_bullet_id = (character_bullet_id + 1) % 1000;
    shape->setName("bullet-character$"+std::to_string(character_bullet_id));
    unordered_map<int, std::vector<Entity *>> em = client->getEntityMap();
    em[client->id].push_back(shape);
    client->setEntityMap(em);
}

void createBulletEnemy(int x, int y, std::vector<Entity*>& E, int id) {
    SDL_Color shapeColor = {200, 80, 80, 255};
    Entity *shape = new Entity( x, y, 15, 23, shapeColor);
    EnemyBulletMovementHandler *eb = new EnemyBulletMovementHandler();
    eb->enemyId = id;
    shape->patternHandler = eb;
    shape->renderingHandler = new DefaultRenderer();
    enemy_bullet_id = (enemy_bullet_id + 1) % 1000;
    shape->setName("bullet-enemy$"+std::to_string(enemy_bullet_id));
    server->entityMap[-1].push_back(shape);

}

void createEnemyCharacter(int x, int y, std::vector<Entity*>& E, int id) {
    // select a random color
    uint8_t r = 200;
    uint8_t g = rand() % 80;
    uint8_t b = rand() % 80;
    SDL_Color shapeColor = {r, g, b, 255};
    Entity* shape = new Entity(x, y, 200, 50, shapeColor);
    std::string idStr = std::to_string(id);
    shape->setName("Enemy-"+idStr);
    shape->renderingHandler = new DefaultRenderer();
    E.push_back(shape);

    // raise an event to shoot a bullet at random time

    Event *shootBulletEnemy = new Event("ShootBulletEnemy");
    eventMap["enemy-" + idStr] = shootBulletEnemy;
    eventMap["enemy-" + idStr]->addParameter("x", x+100);
    eventMap["enemy-" + idStr]->addParameter("y", y+50);
    eventMap["enemy-" + idStr]->addParameter("id", id);

    // randomly choose between 5 and 15
    int randomTime = rand() % 10 + 5;
    eventManager->raiseEvent(shootBulletEnemy, eventManager->timeline->getTime() + randomTime);
}

//////////////////////////////////////// ANIMATION HANDLERS ////////////////////////////////////////
void BulletMovementHandler::moveToPath(Entity *entity, int factor) {
    if (this->patternHandlerTimeline->isParentPaused()) return;

    int64_t currentTime = patternHandlerTimeline->getTime();

    if (this->start_time == -1) {
        this->start_time = currentTime;
    }

    int timeDiff = int(currentTime - this->start_time);
    this->start_time = currentTime;

    entity->y = entity->y - factor;

    if(entity->y < 50){
        // kill the entity. Find the entity in E and remove it from there. After that delete the object too
        for(auto e : client->getEntityMap()[client->id]){
            if(e == entity){
                vector<Entity*> temp = client->getEntityMap()[client->id];
                temp.erase(std::remove(temp.begin(), temp.end(), e), temp.end());
                unordered_map<int, std::vector<Entity *>> em = client->getEntityMap();
                em[client->id] = temp;
                client->setEntityMap(em);
                delete entity;
                break;
            }
        }
    }
    
}

void EnemyBulletMovementHandler::moveToPath(Entity *entity, int factor) {
    if (this->patternHandlerTimeline->isParentPaused()) return;

    int64_t currentTime = patternHandlerTimeline->getTime();

    if (this->start_time == -1) {
        this->start_time = currentTime;
    }

    int timeDiff = int(currentTime - this->start_time);
    this->start_time = currentTime;

    entity->y = entity->y + factor*2;

    if(entity->y > SCREEN_HEIGHT - 10){
        // kill the entity. Find the entity in E and remove it from there. After that delete the object too
        int randomTime = rand() % 10 + 5;
        eventManager->raiseEvent(eventMap["enemy-" + std::to_string(enemyId)], eventManager->timeline->getTime() + randomTime);

        for(auto e : server->entityMap[-1]){
            if(e == entity){
                vector<Entity*> temp = server->entityMap[-1];
                temp.erase(std::remove(temp.begin(), temp.end(), e), temp.end());
                server->entityMap[-1] = temp;
                delete entity;
                break;
            }
        }
    }
}
//////////////////////////////////////// PHYSICS HANDLERS ////////////////////////////////////////

void XPhysicsHandler::handleInput(Entity *entity) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    bool parent_paused = this->physicsTimeline->isParentPaused();
    if (parent_paused) return;

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

    // If the 'W' key is pressed
    if (state[SDL_SCANCODE_W] && !this->PState[SDL_SCANCODE_W]) {
        // shoot a bullet
        Event *shootBulletCharacter = new Event("ShootBulletCharacter");
        shootBulletCharacter->addParameter("x", entity->x+50);
        shootBulletCharacter->addParameter("y", entity->y-50);
        eventManager->raiseEvent(shootBulletCharacter, 0);
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

    int xbound = SCREEN_WIDTH;

    if (entity->x + entity->w + locDifference >= xbound) {
        entity->x = xbound - entity->w;
    }else if(entity->x + locDifference <= 0){
        entity->x = 0;
    }else {
        entity->x += locDifference;
    }
}

//////////////////////////////////////// COLLISION HANDLERS ////////////////////////////////////////

void CharacterCollisionHandler::triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) {
    if(entity->getName() == "Character"){
        for (const auto pair : entityMap) {
            std::vector<Entity *> entities = pair.second;
            for (Entity *other : entities) {
                // split other->getName() by $
                vector<string> tokens = split(other->getName(), '$');
                if(tokens.size() == 2){
                    if(tokens[0] == "bullet-enemy" && entity->checkCollision(*other)){
                        // send a message to the server to kill the bullet and reduce the health of the character
                        string msg = tokens[1];
                        string message = client->messageHandler.createMessage(5, msg);
                        client->messageHandler.sendMessage(client->push_pull, message);
                        health -= 20;

                        if (health <= 0) {
                            gameTimeline->pause();
                            gameOverText->setText("Game Over");
                            // send message to server to pause timeline
                            msg = "pause";
                            message = client->messageHandler.createMessage(6, msg);
                            client->messageHandler.sendMessage(client->push_pull, message);
                        }
                    }
                }
            }
        }
    }
}

void CharacterBulletCollisionHandler::triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) {
    vector<string> tokens = split(entity->getName(), '$');
    if(tokens[0] == "bullet-character"){
        for (const auto pair : entityMap) {
            std::vector<Entity *> entities = pair.second;
            for (Entity *other : entities) {
                tokens = split(other->getName(), '-');
                if(tokens.size() == 2){
                    if(tokens[0] == "Enemy" && entity->checkCollision(*other)){
                        // kill the bullet
                        for(auto e : entityMap[client->id]){
                            if(e == entity){
                                vector<Entity*> temp = entityMap[client->id];
                                temp.erase(std::remove(temp.begin(), temp.end(), e), temp.end());
                                entityMap[client->id] = temp;
                                delete e;
                                break;
                            }
                        }

                        // send message to server to kill the enemy
                        string msg = tokens[1];
                        string message = client->messageHandler.createMessage(7, msg);
                        client->messageHandler.sendMessage(client->push_pull, message);

                        score += 10;
                    }
                }

                tokens = split(other->getName(), '$');
                if(tokens.size() == 2){
                    if(tokens[0] == "bullet-enemy" && entity->checkCollision(*other)){
                        // kill the bullet
                        for(auto e : entityMap[client->id]){
                            if(e == entity){
                                vector<Entity*> temp = entityMap[client->id];
                                temp.erase(std::remove(temp.begin(), temp.end(), e), temp.end());
                                entityMap[client->id] = temp;
                                delete e;
                                break;
                            }
                        }

                        // send message to server to kill the bullet
                        string msg = tokens[1];
                        string message = client->messageHandler.createMessage(5, msg);
                        client->messageHandler.sendMessage(client->push_pull, message);
                    }
                }
            }
        }
    }
}


//////////////////////////////////////// EVENTS ////////////////////////////////////////
void create_events(){
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

    eventManager->registerEvent("GoRightEvent", new GoRightEventHandler());
    eventManager->registerEvent("GoLeftEvent", new GoLeftEventHandler());
    eventManager->registerEvent("ShootBulletCharacter", new ShootBulletCharacterEventHandler());
    eventManager->registerEvent("ShootBulletEnemy", new ShootBulletEnemyEventHandler());
}

//////////////////////////////////////// EVENT HANDLERS ////////////////////////////////////////
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

void ShootBulletCharacterEventHandler::onEvent(Event e) {
    if(e.type == "ShootBulletCharacter"){
        int x = e.getParameter("x")->m_asInt;
        int y = e.getParameter("y")->m_asInt;
        createBulletCharacter(x, y, E);
    }
}

void ShootBulletEnemyEventHandler::onEvent(Event e) {
    if(e.type == "ShootBulletEnemy"){
        // check if enemy is still alive
        for(auto entity : server->entityMap[-1]){
            vector<string> tokens = split(entity->getName(), '-');
            if(tokens.size() == 2){
                if(tokens[0] == "Enemy" && stoi(tokens[1]) == e.getParameter("id")->m_asInt){
                    int x = e.getParameter("x")->m_asInt;
                    int y = e.getParameter("y")->m_asInt;
                    createBulletEnemy(x, y, E, e.getParameter("id")->m_asInt);
                    break;
                }
            }
        }

        // int x = e.getParameter("x")->m_asInt;
        // int y = e.getParameter("y")->m_asInt;
        // int id = e.getParameter("id")->m_asInt;
        // createBulletEnemy(x, y, E, id);
    }
}


///////////////////////////////////////////////////////// UTILS /////////////////////////////////////////////////////////////////////////////////////////////////////////

void checkGameOver() {
    // check if all enemies are dead
    unordered_map<int, std::vector<Entity *>> em = client->getEntityMap();
    if (em[-1].size() == 0) {
        gameTimeline->pause();
        gameOverText->setText("Y O U  W I N");
        // send message to server to pause timeline
        string msg = "pause";
        string message = client->messageHandler.createMessage(6, msg);
        client->messageHandler.sendMessage(client->push_pull, message);
    }
}

void HandleGentleExit(Client *client){
    if(app->quit){
        // Event *sendServerEvent = new Event("PlayerExitEvent");
        // sendServerEvent->addParameter("Client Id", client->id);
        // string msg = client->serializer.serializeEvent(sendServerEvent, eventManager->timeline->getTime()+2);
        // string message = client->messageHandler.createMessage(0, msg);
        // client->messageHandler.sendMessage(client->push_pull, message);
        SDL_Quit();
        exit(0);
    }
}

// CUSTOM SERVER MESSAGE HANDLING
void CustomServer::handleCustomRequest(string message) {
    auto msg = messageHandler.parseMessage(message);
    int type = msg.first;
    string data = msg.second;

    if (type == 5) {
        // kill the bullet and reduce the health of the character
        int id = stoi(data);
        for(auto e : entityMap[-1]){
            vector<string> tokens = split(e->getName(), '$');
            if(tokens.size() == 2){
                if(tokens[0] == "bullet-enemy" && stoi(tokens[1]) == id){
                    vector<Entity*> temp = entityMap[-1];
                    temp.erase(std::remove(temp.begin(), temp.end(), e), temp.end());
                    entityMap[-1] = temp;
                    delete e;
                    break;
                }
            }
        }
    }else if (type == 6) {
        // pause the timeline
        gameTimeline->pause();
    }else if (type == 7) {
        // kill the enemy
        int id = stoi(data);
        for(auto e : entityMap[-1]){
            vector<string> tokens = split(e->getName(), '-');
            if(tokens.size() == 2){
                if(tokens[0] == "Enemy" && stoi(tokens[1]) == id){
                    vector<Entity*> temp = entityMap[-1];
                    temp.erase(std::remove(temp.begin(), temp.end(), e), temp.end());
                    entityMap[-1] = temp;
                    delete e;
                    break;
                }
            }
        }
    }
}

vector<string> split(string s, char delimiter) {
    vector<string> tokens;
    stringstream check1(s);
    string intermediate;
    while(getline(check1, intermediate, delimiter)) {
        tokens.push_back(intermediate);
    }
    return tokens;
}