#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <unordered_map>

// Forward declarations
class Entity;
class Timeline;

struct variant {
    enum Type {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_GAMEOBJECT,
        TYPE_DOUBLE,
        TYPE_ENTITYMAP,
        TYPE_TIMELINE,
    } m_Type;

    union {
        int m_asInt;
        float m_asFloat;
        Entity* m_asGameObject;
        double m_asDouble;
        std::unordered_map<int, std::vector<Entity *>> *m_asEntityMap;
        Timeline* m_asTimeline;
    };

    // Constructors
    variant() : m_Type(TYPE_INT), m_asInt(0) {}
    variant(int val) : m_Type(TYPE_INT), m_asInt(val) {}
    variant(float val) : m_Type(TYPE_FLOAT), m_asFloat(val) {}
    variant(double val) : m_Type(TYPE_DOUBLE), m_asDouble(val) {}
    variant(Entity* gameObj) : m_Type(TYPE_GAMEOBJECT), m_asGameObject(gameObj) {}
    variant(std::unordered_map<int, std::vector<Entity *>> *entityMap) : m_Type(TYPE_ENTITYMAP), m_asEntityMap(entityMap) {}
    variant(Timeline *timeline) : m_Type(TYPE_TIMELINE), m_asTimeline(timeline) {}
};

class Event {
public: 
    size_t typeHash;
    std::string type;
    std::map<std::string, variant> parameters;

    Event(const std::string& eventType);
    void addParameter(const std::string& paramName, const variant& value);
    const variant* getParameter(const std::string& paramName) const;
};

class EventHandler {
public:
    virtual void onEvent(Event e) = 0;
    virtual ~EventHandler() = default;
};