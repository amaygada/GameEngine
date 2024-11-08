#include "event_manager.hpp"

Event::Event(const std::string& eventType) {
    std::hash<std::string> hasher;
    typeHash = hasher(eventType);
    type = eventType;
}

void Event::addParameter(const std::string& paramName, const variant& value) {
    parameters[paramName] = value;
}

const variant* Event::getParameter(const std::string& paramName) const {
    auto it = parameters.find(paramName);
    if (it != parameters.end()) {
        return &(it->second);
    }
    return nullptr;
}

EventManager::EventManager(Timeline *eventManagerTimeline) {
    timeline = eventManagerTimeline;
}

void EventManager::registerEvent(const std::string& eventType, EventHandler* handler) {
    eventHandlers[eventType].push_back(handler);
}

void EventManager::deregisterEvent(const std::string& eventType, EventHandler* handler) {
    auto it = eventHandlers.find(eventType);
    if (it != eventHandlers.end()) {
        std::vector<EventHandler*>& handlers = it->second;
        for (auto it = handlers.begin(); it != handlers.end(); ++it) {
            if (*it == handler) {
                handlers.erase(it);
                return;
            }
        }
    }
}

void EventManager::raiseEvent(Event* event, int time, Client *client) {
    std::lock_guard<std::mutex> lock(this->mtx);
    this->eventQueue.push({time, event});
    
    client->pushEvent(event, time);
}
