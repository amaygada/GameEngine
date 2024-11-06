#include "event.hpp"

Event *quitGameEvent = new Event("QuitGameEvent");
Event *changeTicEvent = new Event("ChangeTicEvent");
Event *gamePauseEvent = new Event("GamePauseEvent");
Event *defaultCollisionEvent = new Event("DefaultCollisionEvent");

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
        std:cout << "Collision detected between Entity " << entity2 << " and Entity " << entity << std::endl;        
    }
}