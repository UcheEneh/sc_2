#pragma once

#include <string>

#include <unordered_map>
#include <vector>
#include <queue>

#include "Event.h"
#include "GameObject.h"

using GameObjMap = std::unordered_map<std::string, std::pair<std::vector<GameObject>, int> >;

using EventQueue = std::priority_queue<Event, std::vector<Event>, std::greater<Event> >;

//interface for subscriber
class subscriber
{
public:
    virtual void notify() = 0;

    //remember the using type aliases
    //hand over game objects and events
    //if nothing is done, create dummy Event

    virtual Event doSpecialThings(GameObjMap& game_objects, EventQueue& event_list, int glob_time, int events_occured) = 0;
};
