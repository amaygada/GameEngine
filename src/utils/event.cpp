#include "event.hpp"

Event *quitGameEvent = new Event("QuitGameEvent");
Event *changeTicEvent = new Event("ChangeTicEvent");
Event *gamePauseEvent = new Event("GamePauseEvent");
Event *defaultCollisionEvent = new Event("DefaultCollisionEvent");
Event *defaultPhysicsEventRight = new Event("DefaultPhysicsEvent");
Event *defaultPhysicsEventLeft = new Event("DefaultPhysicsEvent");
Event *defaultPhysicsEventUp = new Event("DefaultPhysicsEvent");
Event *defaultPhysicsEventDown = new Event("DefaultPhysicsEvent");
Event *recordEvent = new Event("RecordEvent");

void addParametersEvent(){
    defaultPhysicsEventRight->addParameter("velocityX", double(1));
    defaultPhysicsEventRight->addParameter("velocityY", double(0));
    defaultPhysicsEventRight->addParameter("accelerationX", double(1));
    defaultPhysicsEventRight->addParameter("accelerationY", double(0));
    defaultPhysicsEventRight->addParameter("direction", 1);

    defaultPhysicsEventLeft->addParameter("velocityX", double(1));
    defaultPhysicsEventLeft->addParameter("velocityY", double(0));
    defaultPhysicsEventLeft->addParameter("accelerationX", double(1));
    defaultPhysicsEventLeft->addParameter("accelerationY", double(0));
    defaultPhysicsEventLeft->addParameter("direction", -1);

    defaultPhysicsEventUp->addParameter("velocityX", double(0));
    defaultPhysicsEventUp->addParameter("velocityY", double(1));
    defaultPhysicsEventUp->addParameter("accelerationX", double(0));
    defaultPhysicsEventUp->addParameter("accelerationY", double(1));
    defaultPhysicsEventUp->addParameter("direction", -1);

    defaultPhysicsEventDown->addParameter("velocityX", double(0));
    defaultPhysicsEventDown->addParameter("velocityY", double(1));
    defaultPhysicsEventDown->addParameter("accelerationX", double(0));
    defaultPhysicsEventDown->addParameter("accelerationY", double(1));
    defaultPhysicsEventDown->addParameter("direction", 1);
}

void QuitGameEventHandler::onEvent(Event e) {
    app->quit = true;
}

void ChangeTicEventHandler::onEvent(Event e) {
    if(e.type == "ChangeTicEvent"){
        gameTimeline->changeTic(e.getParameter("tic")->m_asDouble);
    }
}

void GamePauseEventHandler::onEvent(Event e) {
    if(e.type == "GamePauseEvent"){
        if(gameTimeline->isPaused()) gameTimeline->resume();
        else gameTimeline->pause();
    }
}

void DefaultCollisionEventHandler :: onEvent(Event e) {
    if(e.type == "DefaultCollisionEvent"){
        const variant *entityMapVariant = e.getParameter("EntityMap");
        Entity *entity = e.getParameter("Entity1")->m_asGameObject;
        Entity *entity2 = e.getParameter("Entity2")->m_asGameObject;
        cout << "Collision detected between Entity " << entity2 << " and Entity " << entity << std::endl;        
    }
}

void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
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

    to_be_loc = velocity_y + (0.5 * acceleration_y * (timeValue * timeValue));
    locDifference = direction * (to_be_loc);

    if (entity->y + entity->h + locDifference >= SCREEN_HEIGHT) {
        entity->y = SCREEN_HEIGHT - entity->h;
    }else if(entity->y + locDifference <= 0){
        entity->y = 0;
    }else {
        entity->y += locDifference;
    }
}

void DefaultPhysicsEventHandler :: onEvent(Event e) {
    if(e.type == "DefaultPhysicsEvent"){
        Entity *entity = e.getParameter("Entity")->m_asGameObject;
        double velocity_x = e.getParameter("velocityX")->m_asDouble;
        double velocity_y = e.getParameter("velocityY")->m_asDouble;
        double acceleration_x = e.getParameter("accelerationX")->m_asDouble;
        double acceleration_y = e.getParameter("accelerationY")->m_asDouble;
        int direction = e.getParameter("direction")->m_asInt;
        updatePhysics(entity, velocity_x, velocity_y, acceleration_x, acceleration_y, direction);
    }
}

void clearFile(const std::string &filename) {
    std::ofstream file(filename, std::ios::out); // Open in output mode
    if (file.is_open()) {
        file.close(); // Closing immediately clears the file
    } else {
        std::cerr << "Failed to open the file: " << filename << std::endl;
    }
}

void RecordEventHandler :: onEvent(Event e) {
    if(e.type == "RecordEvent"){
        app->record = !app->record;
        if(app->record){
            clearFile("record.txt");
        }else{
            moving = false;
            app->replay = true;
            gameTimeline->pause();
        }
    }
}