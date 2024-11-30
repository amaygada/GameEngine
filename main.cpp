#include "main.hpp"

std::map<std::string, Event*> eventMap;
vector<Entity*> E;

CustomServer *server;
Client *client;

bool moving = false;
int blockId = 0;
double vx = 0.8;
double vy = 0.8;
int curr_x = 0;
int curr_y = 0;
int direction = 1;
bool shootAllowed = false;
int score1 = 0;
int health1 = 100;
int score2 = 0;
int health2 = 100;

bool sa1 = true;
bool sa2 = false;

Text *vxText = nullptr;
Text *vyText = nullptr;
Text *p1Text = nullptr;
Text *p2Text = nullptr;
Text *p1Score = nullptr;
Text *p2Score = nullptr;
Text *p1Health = nullptr;
Text *p2Health = nullptr;
Text *p1Win = nullptr;
Text *p2Win = nullptr;

bool flag = false;

void create_events();
void createDeathZone(std::vector<Entity*>& E);
void HandleGentleExit(Client *client);

void createBuilding(std::vector<Entity*>& E, int starting_point, int width, int height);
void createClientCharacter(int x, int y, std::vector<Entity*>& E);
void createBanana(std::vector<Entity*>& E, int x, int y);

vector<string> split(string s, char delimiter);

void run_client_server(bool isServer) {
    if(isServer){
        server = new CustomServer(); 

        createBuilding(E, 0, 10, 15);
        createBuilding(E, 210, 15, 25);
        createBuilding(E, 500, 10, 20);
        createBuilding(E, 710, 16, 36);
        createBuilding(E, 1040, 10, 24);
        createBuilding(E, 1250, 15, 20);
        createBuilding(E, 1540, 10, 15);
        createBuilding(E, 1750, 8, 25);

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
        createDeathZone(E);
        
        client->performHandshake(E);

        if(client->id == 0)       {createClientCharacter(50, SCREEN_HEIGHT-360, E); curr_x = 50; curr_y = SCREEN_HEIGHT-360; shootAllowed = true;}
        else if(client->id == 1) {createClientCharacter(1800, SCREEN_HEIGHT-560, E); curr_x = 1800; curr_y = SCREEN_HEIGHT-560; direction=-1;}
        else {cout<<"Only 2 clients allowed"; exit(0);}

        unordered_map<int, std::vector<Entity *>> em = client->getEntityMap();
        em[client->id] = E;
        client->setEntityMap(em);

        renderer->init("Game");

        // Main game loop for client
        int64_t last_render_time = globalTimeline->getTime();
        while(true) {
            int64_t frame_delta = globalTimeline->getTime() - last_render_time;

            if (frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);
            last_render_time = globalTimeline->getTime();

            if(flag) {gameTimeline->pause();}

            auto msg = client->messageHandler.createMessage(8, "scores/health");
            client->messageHandler.sendMessage(client->req_rep, msg);
            string reply = client->messageHandler.receiveMessage(client->req_rep);
            auto scores = client->messageHandler.parseMessage(reply);
            vector<string> tokens = split(scores.second, '/');
            score1 = stoi(tokens[0]);
            score2 = stoi(tokens[1]);
            health1 = stoi(tokens[2]);
            health2 = stoi(tokens[3]);

            if(health1 <= 0) {p2Win->setText("Y o u  W i n"); flag=true;}
            if(health2 <= 0) {p1Win->setText("Y o u  W i n"); flag=true;}

            vxText->setText("V E L O C I T Y   X  : " + std::to_string(vx));
            vyText->setText("V E L O C I T Y   Y  :  " + std::to_string(vy));
            p1Score->setText("Score: " + std::to_string(score1));
            p2Score->setText("Score: " + std::to_string(score2));
            p1Health->setText("Health: " + std::to_string(health1));
            p2Health->setText("Health: " + std::to_string(health2));

            renderer->prepareScene();
            vxText->renderText();
            vyText->renderText();
            p2Win->renderText();
            p1Win->renderText();
            if(client->id == 0 && shootAllowed && !flag) {p1Text->setText("YOUR TURN"); p2Text->setText(""); p1Text->renderText(); p2Text->renderText();}  
            else if(client->id == 1 && shootAllowed && !flag) { p2Text->setText("YOUR TURN"); p1Text->setText(""); p2Text->renderText(); p1Text->renderText();}
            
            if (client->id == 0 && !shootAllowed && !flag) {p1Text->setText(""); p2Text->setText("YOUR TURN"); p1Text->renderText(); p2Text->renderText();}
            else if (client->id == 1 && !shootAllowed && !flag) {p1Text->setText("YOUR TURN"); p2Text->setText(""); p1Text->renderText(); p2Text->renderText();}


            p1Score->renderText();
            p2Score->renderText();
            p1Health->renderText();
            p2Health->renderText();


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

             //just once

            // Client-server communication
            client->sendEntityUpdate();       // Client sends its entity update to the server
            client->receiveEntityUpdates();  // Client receives entity updates from the server
            
            renderer->presentScene(client->getEntityMap(), client->id, false);
        }

        renderer->cleanup();
    }
}

int main(int argc, char *argv[]){
    srand(time(0));
    app->quit = false;
    app->displacement = 0;

    create_events();
    vxText = new Text("VelocityX: 0", 950, 110, 300, 30, {0, 0, 0, 255});
    vyText = new Text("VelocityY: 0", 950, 160, 300, 30, {0, 0, 0, 255});

    p1Text = new Text("Your turn", 100, 50, 200, 30, {0, 0, 0, 255});
    p2Text = new Text("Your turn", 1500, 50, 200, 30, {0, 0, 0, 255});
    p1Score = new Text("Score: 0", 100, 100, 200, 30, {0, 0, 0, 255});
    p2Score = new Text("Score: 0", 1500, 100, 200, 30, {0, 0, 0, 255});
    p1Health = new Text("Health: 100", 100, 150, 200, 30, {0, 0, 0, 255});
    p2Health = new Text("Health: 100", 1500, 150, 200, 30, {0, 0, 0, 255});
    p1Win = new Text("", 100, 200, 200, 30, {0, 0, 0, 255});
    p2Win = new Text("", 1500, 200, 200, 30, {0, 0, 0, 255});

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

void createBuilding(std::vector<Entity*>& E, int starting_point, int width, int height) {
    int dim = 40;
    for(int i = 0; i < width/2; i+=1) {
        for(int j = 0; j < height/2; j+=1) {
            int col = 130 + rand()%125;
            SDL_Color shapeColor = {col, col, col, 255};
            Entity *shape = new Entity(starting_point+(i*dim), SCREEN_HEIGHT-(j*dim), dim, dim, shapeColor);
            shape->renderingHandler = new DefaultRenderer();
            shape->setName("Building$"+to_string(blockId));
            E.push_back(shape);
            blockId++;
        }
    }
}

void createClientCharacter(int x, int y, std::vector<Entity*>& E) {
    // select a random color
    uint8_t r = rand() % 256;
    uint8_t g = rand() % 256;
    uint8_t b = 255;
    SDL_Color shapeColor = {r, g, b, 255};
    Entity* shape = new Entity(x, y, 60, 120, shapeColor);
    shape->setName("Character$"+to_string(client->id));
    shape->renderingHandler = new DefaultRenderer();
    shape->inputHandler = new CharacterInputHandler();
    E.push_back(shape);  // Push the dynamically allocated entity
}

void createBanana(std::vector<Entity*>& E, int x, int y) {
    SDL_Color shapeColor = {255, 255, 0, 255};  // Yellow color
    Entity *shape = new Entity( x, y, 100, 20, shapeColor);
    shape->renderingHandler = new DefaultRenderer();
    shape->collisionHandler = new BananaCollisionHandler();
    shape->patternHandler = new BananaAnimationHandler();
    shape->setName("Banana$"+std::to_string(client->id));
    E.push_back(shape);
}


//////////////////////////////////////// ANIMATION HANDLERS /////////////////////////////////////////
void BananaAnimationHandler::moveToPath(Entity *entity, int factor) {
    if (this->patternHandlerTimeline->isParentPaused()) return;

    if (this->start_time == -1) {
        this->start_time = patternHandlerTimeline->getTime();
    }
    if(entity->x >= SCREEN_WIDTH || entity->x <= 0){
       return;
    }
    if(entity->y >= SCREEN_HEIGHT || entity->y <= 0){
        return;
    }

    int timeValue = int(patternHandlerTimeline->getTime() - this->start_time);

    int current_loc = entity->x;
    if(timeValue == 0) current_loc = curr_x;
    int to_be_loc = curr_x + direction*(vx * timeValue * 15);
    int locDifference = to_be_loc - current_loc;
    if(timeValue == 0) entity->x = curr_x + locDifference;
    else entity->x += locDifference;

    double tValue = (double)timeValue/2;
    current_loc = entity->y;
    if(tValue == 0) current_loc = curr_y;
    to_be_loc = curr_y - (vy * timeValue * 15) + (0.5 * 1 * (tValue * tValue));
    locDifference = to_be_loc - current_loc;
    if(tValue == 0) entity->y = curr_y + locDifference;
    else entity->y += locDifference;
}

//////////////////////////////////////// PHYSICS HANDLERS /////////////////////////////////////////


//////////////////////////////////////// INPUT HANDLERS ///////////////////////////////////////////
void CharacterInputHandler::handleInput(Entity *entity) {
    if(gameTimeline->isPaused()) return;
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    string message = client->messageHandler.createMessage(5, std::to_string(client->id));
    client->messageHandler.sendMessage(client->req_rep, message);
    string reply = client->messageHandler.receiveMessage(client->req_rep);
    auto msg = client->messageHandler.parseMessage(reply);

    if (msg.first == 5) {
        if (msg.second == "1") shootAllowed = true;
        else shootAllowed = false;
    }

    if(shootAllowed == false) return;

    // If the 'D' key is pressed
    if (state[SDL_SCANCODE_D] && !this->PState[SDL_SCANCODE_D]) {
        // increase velocity in x
        if(vx < 1)  vx += 0.05;
    }else if (state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        // do nothing
    }else if (!state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        // do nothing
    }

    // If the 'W' key is pressed
    if (state[SDL_SCANCODE_W] && !this->PState[SDL_SCANCODE_W]) {
        // increase velocity in y
        if(vy < 1) vy += 0.05;
    }else if (state[SDL_SCANCODE_W] && this->PState[SDL_SCANCODE_W]) {
        // do nothing
    }else if (!state[SDL_SCANCODE_W] && this->PState[SDL_SCANCODE_W]) {
        // do nothing
    }

    // If the 'S' key is pressed
    if (state[SDL_SCANCODE_S] && !this->PState[SDL_SCANCODE_S]) {
        // decrease velocity in y
        if(vy > 0) vy -= 0.05;
    }else if (state[SDL_SCANCODE_S] && this->PState[SDL_SCANCODE_S]) {
        // do nothing
    }else if (!state[SDL_SCANCODE_S] && this->PState[SDL_SCANCODE_S]) {
        // do nothing
    }

    // If the 'A' key is pressed
    if (state[SDL_SCANCODE_A] && !this->PState[SDL_SCANCODE_A]) {
        // decrease velocity in x
        if(vx > 0) vx -= 0.05;
    }else if (state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        // do nothing
    }else if (!state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        // do nothing
    }

    // If Enter is pressed
    if (state[SDL_SCANCODE_RETURN] && !this->PState[SDL_SCANCODE_RETURN]) {
        // create the banana at the current position of the character
        if(vx == 0 && vy == 0) return;
        createBanana(E, curr_x, curr_y);  
        unordered_map<int, std::vector<Entity *>> em = client->getEntityMap();
        em[client->id] = E;
        client->setEntityMap(em);
        shootAllowed = false;
        string message = client->messageHandler.createMessage(6, std::to_string(client->id));
        client->messageHandler.sendMessage(client->push_pull, message);

        // tell server that player 2 can shoot banana now.
        
    }else if (state[SDL_SCANCODE_RETURN] && this->PState[SDL_SCANCODE_RETURN]) {
        // do nothing
    }else if (!state[SDL_SCANCODE_RETURN] && this->PState[SDL_SCANCODE_RETURN]) {
        // do nothing
    }

    for (int i = 0; i < 512; i++) {
        this->PState[i] = state[i];
    }
}

//////////////////////////////////////// COLLISION HANDLERS /////////////////////////////////////////
void BananaCollisionHandler::triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) {
    if(!entity) return;
    vector<string> tokens = split(entity->getName(), '$');
    int idd = 1 ? client->id == 0 : 0;
    if (tokens.size() == 2){
        int banana_id = stoi(tokens[1]);
        if(tokens[0] == "Banana"){
            for (const auto pair : entityMap) {
                std::vector<Entity *> entities = pair.second;
                for (Entity *other : entities) {
                    tokens = split(other->getName(), '$');
                    if(tokens.size() == 2){
                        if(tokens[0] == "Building" && entity->checkCollision(*other)){
                            // send message to server to remove Building block
                            string msg = client->messageHandler.createMessage(7, tokens[1]);
                            client->messageHandler.sendMessage(client->push_pull, msg);
                            msg = client->messageHandler.createMessage(9, std::to_string(client->id));
                            client->messageHandler.sendMessage(client->push_pull, msg);

                            Event *deleteBananaEvent = new Event("DeleteBananaEvent");
                            deleteBananaEvent->addParameter("entity", entity);
                            deleteBananaEvent->addParameter("opt", 1);
                            eventManager->raiseEvent(deleteBananaEvent, eventManager->timeline->getTime());
                        }
                    }
                    if(other->getName() == "Character$"+to_string(idd) && entity->checkCollision(*other)){
                        Event *deleteBananaEvent = new Event("DeleteBananaEvent");
                        deleteBananaEvent->addParameter("entity", entity);
                        deleteBananaEvent->addParameter("opt", 0);
                        eventManager->raiseEvent(deleteBananaEvent, eventManager->timeline->getTime());
                    }
                }
            }
        }
    }
}


//////////////////////////////////////// EVENTS /////////////////////////////////////////////////////
void create_events(){
    eventManager->registerEvent("DeleteBananaEvent", new DeleteBananaEventHandler());
}

//////////////////////////////////////// EVENT HANDLERS /////////////////////////////////////////////

void DeleteBananaEventHandler::onEvent(Event e) {

    Entity *entity = e.getParameter("entity")->m_asGameObject;
    int opt = e.getParameter("opt")->m_asInt;
    int idd = 1 ? client->id == 0 : 0;
    if(entity == nullptr) return;
    auto em = client->getEntityMap();
    // remove banana from em[client->id]
    for (int i = 0; i < em[client->id].size(); i++) {
        if (em[client->id][i] == entity) {
            em[client->id].erase(em[client->id].begin() + i);
            delete entity;
            if(opt == 0){
                string msg = client->messageHandler.createMessage(10, std::to_string(idd));
                client->messageHandler.sendMessage(client->push_pull, msg);
            }
            break;
        }
    }
    client->setEntityMap(em);


}


//////////////////////////////////////// UTILS //////////////////////////////////////////////////////

void HandleGentleExit(Client *client){
    if(app->quit){
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
        int id = stoi(data);
        if (id == 0){
            std::string shootAllowedStr = sa1 ? "1" : "0";
            string message = server->messageHandler.createMessage(5, sa1 ? "1" : "0");
            server->messageHandler.sendMessage(server->req_rep, message);
        }
        else if (id == 1){
            string message = server->messageHandler.createMessage(5, sa2 ? "1" : "0");
            server->messageHandler.sendMessage(server->req_rep, message);
        };
    } else if (type == 6) {
        int id = stoi(data);
        sa1 = !sa1;
        sa2 = !sa2;
    } else if (type == 7) {
        int blockId = stoi(data);
        for (int i = 0; i < server->entityMap[-1].size(); i++) {
            if (server->entityMap[-1][i]->getName() == "Building$"+data) {
                auto *entity = server->entityMap[-1][i];
                server->entityMap[-1].erase(server->entityMap[-1].begin() + i);
                break;
            }
        }
    } else if (type == 8) {
        string message = server->messageHandler.createMessage(8, std::to_string(score1)+"/"+std::to_string(score2)+"/"+std::to_string(health1)+"/"+std::to_string(health2));
        server->messageHandler.sendMessage(server->req_rep, message);
    } else if (type == 9) {
        int id = stoi(data);
        if (id == 0) score1 += 1;
        else score2 += 1;
    } else if (type == 10) {
        int id = stoi(data);
        if (id == 0) {health1 -= 50; score2 += 10;}
        else {health2 -= 50; score1 += 10;}
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