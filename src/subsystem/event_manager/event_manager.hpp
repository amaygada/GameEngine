#pragma once
#include "event_base.hpp"
#include <mutex>
#include <queue>
#include "./../connection/client.hpp"

class EventManager {
private:
    std::mutex mtx;
    struct EventComparator {
        bool operator()(
            const std::pair<int, Event*>& a,
            const std::pair<int, Event*>& b
        ) {
            return a.first > b.first;
        }
    };

public:
    std::map<std::string, std::vector<EventHandler*>> eventHandlers;
    std::priority_queue<std::pair<int, Event*>, 
                       std::vector<std::pair<int, Event*>>, 
                       EventComparator> eventQueue;
    Timeline *timeline;

    EventManager(Timeline *EventManagerTimeline);
    void registerEvent(const std::string& eventType, EventHandler* handler);
    void deregisterEvent(const std::string& eventType, EventHandler* handler);
    void raiseEvent(Event *event, int time = 0, Client *client);
};
