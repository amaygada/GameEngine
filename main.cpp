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
void createClientCharacter(std::vector<Entity*>& E);
void createEnemyCharacter(std::vector<Entity*>& E, int &id);
void createBulletEnemy(int x, int y, std::vector<Entity*>& E, int &id);
void createBulletCharacter(int x, int y, std::vector<Entity*>& E);
void HandleGentleExit(Client *client);
void checkGameOver();
void createBall(std::vector<Entity*>& E);
vector<string> split(string s, char delimiter);

int health = 50;
int score = 0;

Text *healthText = nullptr;
Text *scoreText = nullptr;
Text *gameOverText = nullptr;

void run_client_server(bool isServer) {
    if(isServer){
        server = new CustomServer();

        int temp_d = 0;

        // Cluster 1
        createEnemyCharacter(E, temp_d);
        
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
        createClientCharacter(E);
        createBall(E);

        renderer->init("Game");

        // Main game loop for client
        int64_t last_render_time = globalTimeline->getTime();
        while(true) {
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;

            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            healthText->setText("Health: " + std::to_string(health));
            scoreText->setText("Kills: " + std::to_string(score));

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

    healthText = new Text("Health: 50", (int)SCREEN_WIDTH - (SCREEN_WIDTH/2), 110, 120, 50, {0, 0, 0, 200});
    scoreText = new Text("Score: 0", (int)SCREEN_WIDTH - (SCREEN_WIDTH/2), 150, 120, 50, {0, 0, 0, 200});
    gameOverText = new Text("", (int)SCREEN_WIDTH - (SCREEN_WIDTH/2), (int)SCREEN_HEIGHT - (SCREEN_HEIGHT/2), 240, 100, {0, 0, 0, 200});

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

void createClientCharacter(std::vector<Entity*>& E) {
    int platformWidth = 200;
    int platformHeight = 20;
    // Position the platform near the bottom center of the screen
    int x = (SCREEN_WIDTH - platformWidth) / 2;
    int y = SCREEN_HEIGHT - 100; // Adjust this as needed

    // Choose a color for the platform
    SDL_Color platformColor = {128, 128, 128, 255}; // A shade of gray

    // Create the platform entity
    Entity* platform = new Entity(x, y, platformWidth, platformHeight, platformColor);
    platform->setName("Character");
    platform->renderingHandler = new DefaultRenderer();
    platform->physicsHandler = new XPhysicsHandler(true);
    platform->collisionHandler = new CharacterCollisionHandler();

    // Add the shape to the entity vector
    E.push_back(platform);

    unordered_map<int, std::vector<Entity *>> em = client->getEntityMap();
    em[client->id].push_back(platform);
    client->setEntityMap(em);
}

void createBall(std::vector<Entity*>& E) {
    SDL_Color ballColor = {255, 255, 255, 255}; // White ball
    Entity* ball = new Entity(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 150, 40, 40, ballColor);
    ball->setName("Ball");
    ball->renderingHandler = new DefaultRenderer();
    ball->patternHandler = new BallMovementHandler();
    ball->collisionHandler = new BallCollisionHandler();
    E.push_back(ball);
}



void createBulletEnemy(int x, int y, std::vector<Entity*>& E, int id) {
    SDL_Color shapeColor = {200, 80, 80, 255};
    Entity *shape = new Entity( x, y, 40, 50, shapeColor);
    EnemyBulletMovementHandler *eb = new EnemyBulletMovementHandler();
    
    eb->enemyId = id;
    shape->patternHandler = eb;
    shape->renderingHandler = new DefaultRenderer();
    //shape->collisionHandler = new EnemyCollisionHandler();
    enemy_bullet_id = (enemy_bullet_id + 1) % 1000;
    shape->setName("bullet-enemy$"+std::to_string(enemy_bullet_id)+"#"+std::to_string(id));
    server->entityMap[-1].push_back(shape);

}

void createEnemyCharacter(std::vector<Entity*>& E, int &id) {

    // Configuration for fewer, evenly-spaced bricks
    int rows = 3;
    int cols = 4;
    int brickWidth = 120;   // Larger bricks
    int brickHeight = 30;
    int horizontalGap = 350; // Horizontal space between bricks
    int verticalGap = 200;   // Vertical space between rows

    // Calculate total width of the entire row of bricks with gaps
    int totalBricksWidth = (cols * brickWidth) + (horizontalGap * (cols - 1));
    
    // Center horizontally on the screen
    int startX = (SCREEN_WIDTH - totalBricksWidth) / 2;
    
    // Vertical offset from the top of the screen
    int startY = 30;

    // Create a small grid of bricks, evenly spaced and centered
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            // Compute each brick's position
            int x = startX + c * (brickWidth + horizontalGap);
            int y = startY + r * (brickHeight + verticalGap);

            // Randomize the brick color slightly
            SDL_Color color = {
                (Uint8)(rand() % 128 + 128),
                (Uint8)(rand() % 128 + 128),
                (Uint8)(rand() % 128 + 128),
                255
            };

            Entity* brick = new Entity(x, y, brickWidth, brickHeight, color);
            std::string idStr = std::to_string(id);
            brick->setName("Enemy-"+idStr);
            brick->renderingHandler = new DefaultRenderer();
            brick->collisionHandler = new BallCollisionHandler();
            E.push_back(brick);
            id++;
        }
    }
}

void createBulletCharacter(int x, int y, std::vector<Entity*>& E) {
    SDL_Color shapeColor = {80, 80, 200, 255};
    Entity *shape = new Entity( x, y, 30, 40, shapeColor);
    std::vector<SDL_Rect> shapePath = {};
    shape->patternHandler = new BulletMovementHandler();
    shape->renderingHandler = new DefaultRenderer();
    //shape->collisionHandler = new CharacterBulletCollisionHandler();
    character_bullet_id = (character_bullet_id + 1) % 1000;
    shape->setName("bullet-character$"+std::to_string(character_bullet_id));
    unordered_map<int, std::vector<Entity *>> em = client->getEntityMap();
    em[client->id].push_back(shape);
    client->setEntityMap(em);
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

void EnemyMovementHandler::moveToPath(Entity* entity, int factor) {
    if (this->patternHandlerTimeline->isParentPaused()) return;

    int64_t currentTime = patternHandlerTimeline->getTime();

    if (this->start_time == -1) {
        this->start_time = currentTime;
    }

    int timeDiff = int(currentTime - this->start_time);
    this->start_time = currentTime;

    // Horizontal movement
    entity->x += factor * direction;

    // Screen bounds check
    if (entity->x + entity->w >= SCREEN_WIDTH || entity->x <= 0) {
        // Reverse direction and move down
        direction *= -1;
        entity->y += stepDown;

        // If moving down passes a threshold (e.g., game over zone), handle it
        if (entity->y + entity->h >= SCREEN_HEIGHT) {
            // Handle game over or enemy reaching the bottom
            std::cout << "Enemy reached the bottom!" << std::endl;
            gameTimeline->pause();
            return;
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

void BallMovementHandler::moveToPath(Entity* entity, int factor) {
    if (this->patternHandlerTimeline->isParentPaused()) return;

    int64_t currentTime = patternHandlerTimeline->getTime();
    if (this->start_time == -1) {
        this->start_time = currentTime;
        // Initialize ball velocity
        velocityX = 5;
        velocityY = -5;
    }

    int timeDiff = int(currentTime - this->start_time);
    this->start_time = currentTime;

    // Update ball position
    entity->x += velocityX;
    entity->y += velocityY;

    // Screen bounds collision
    if (entity->x <= 0 || entity->x + entity->w >= SCREEN_WIDTH) {
        velocityX = -velocityX;
    }
    if (entity->y <= 0) {
        velocityY = -velocityY;
    }

    // Bottom screen collision (game over condition)
    if (entity->y + entity->h >= SCREEN_HEIGHT) {
        std::cout << "Ball reached the bottom! Game Over." << std::endl;
        gameTimeline->pause();
        return;
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
    if(entity->getName() == "Character") {
        for (const auto pair : entityMap) {
            std::vector<Entity *> entities = pair.second;
            for (Entity *other : entities) {
                if(other->getName() == "Ball" && entity->checkCollision(*other)) {
                    // Handle paddle-ball collision
                    BallMovementHandler* ballHandler = dynamic_cast<BallMovementHandler*>(other->patternHandler);
                    if (ballHandler) {
                        // Reverse the vertical direction of the ball
                        ballHandler->velocityY = -abs(ballHandler->velocityY);
                        
                        // Adjust horizontal velocity based on where the ball hit the paddle
                        float hitPosition = (other->x + other->w/2) - (entity->x + entity->w/2);
                        float normalizedHitPosition = hitPosition / (entity->w / 2);
                        ballHandler->velocityX = normalizedHitPosition * 5; // Adjust the multiplier for desired effect
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

                        score += 1;
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
                        string message = client->messageHandler.createMessage(8, msg);
                        client->messageHandler.sendMessage(client->push_pull, message);
                    }
                }
            }
        }
    }
}

void BallCollisionHandler::triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) {
    if (entity->getName() == "Ball") {
        BallMovementHandler* ballHandler = dynamic_cast<BallMovementHandler*>(entity->patternHandler);
        if (!ballHandler) return;

        for (auto &pair : entityMap) {
            auto &entities = pair.second;
            for (auto it = entities.begin(); it != entities.end(); ) {
                Entity *other = *it;
                
                // Check if it's an enemy (brick) and collision occurs
                if (other->getName().compare(0, 5, "Enemy") == 0 && entity->checkCollision(*other)) {
                    // Extract the enemy ID
                    vector<string> tokens = split(other->getName(), '-');
                    if (tokens.size() == 2) {
                        int enemyId = stoi(tokens[1]);

                        // Reverse ball's vertical velocity
                        ballHandler->velocityY = -ballHandler->velocityY;
                        
                        // Remove the brick from local entities
                        delete *it;
                        it = entities.erase(it);

                        // Send message to server to remove the enemy
                        string message = client->messageHandler.createMessage(7, to_string(enemyId));
                        client->messageHandler.sendMessage(client->push_pull, message);

                        // Increase score
                        score += 10;

                        // Exit the loop after handling the collision
                        return;
                    }
                } else {
                    ++it;
                }
            }
        }
    }
}

void EnemyCollisionHandler::triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) {
        if (entity->getName().find("Enemy-") != std::string::npos) {
            for (const auto &pair : entityMap) {
                for (Entity *other : pair.second) {
                    if (other->getName() == "Ball" && entity->checkCollision(*other)) {
                        // Brick-ball collision
                        BallMovementHandler* ballHandler = dynamic_cast<BallMovementHandler*>(other->patternHandler);
                        if (ballHandler) {
                            ballHandler->velocityY = -ballHandler->velocityY;
                        }

                        // Remove the brick
                        vector<Entity*> &entities = entityMap[pair.first];
                        entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
                        delete entity;

                        // Increase score (implement scoring system as needed)
                        // score += 10;
                        return;
                    }
                }
            }
        }
    }


//////////////////////////////////////// EVENTS ////////////////////////////////////////
void create_events(){
    Event *goRightEvent = new Event("GoRightEvent");
    goRightEvent->addParameter("velocityX", double(25));
    goRightEvent->addParameter("velocityY", double(0));
    goRightEvent->addParameter("accelerationX", double(0));
    goRightEvent->addParameter("accelerationY", double(0));
    goRightEvent->addParameter("direction", 1);
    eventMap["GoRightEvent"] = goRightEvent;

    Event *goLeftEvent = new Event("GoLeftEvent");
    goLeftEvent->addParameter("velocityX", double(25));
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
        gameOverText->setText("Congratulations, You win");
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
                vector<string> temp2 = split(tokens[1], '#');
                if(tokens[0] == "bullet-enemy" && stoi(temp2[0]) == id){
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
    }else if (type == 8) {
        // kill the bullet and reduce the health of the character
        int id = stoi(data);
        for(auto e : entityMap[-1]){
            vector<string> tokens = split(e->getName(), '$');
            if(tokens.size() == 2){
                vector<string> temp2 = split(tokens[1], '#');
                if(tokens[0] == "bullet-enemy" && stoi(temp2[0]) == id){
                    int enemyId = stoi(temp2[1]);
                    int randomTime = rand() % 10 + 5;
                    eventManager->raiseEvent(eventMap["enemy-" + std::to_string(enemyId)], eventManager->timeline->getTime() + randomTime);
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