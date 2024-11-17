#pragma once
#include "./../../src/utils/app.hpp"
#include "./../../src/subsystem/event_manager/event_base.hpp"

#include <fstream>

void addParametersEvent();

class QuitGameEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class ChangeTicEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class GamePauseEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class DefaultCollisionEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class DefaultPhysicsEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class RecordEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

extern bool moving;