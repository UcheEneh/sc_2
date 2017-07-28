#pragma once

#include <string>

#include <list>
#include <vector>
#include <unordered_map>
#include <queue>

#include "Event.h"
#include "GameObject.h"
#include "Strategy.h"
#include "Subscriber.h"
#include "AbilityStatus.h"

class SpecialAbility : public subscriber
{
protected:
    int game_object_id_ = 0;
    std::string game_object_name_ = "";
    Strategy* strategy_;

    int energy_ = 0;
    int max_energy_ = 0;
    static const int energy_rate_ = 5625; //multiplied by 10000

    AbilityStatus ability_status_ = AbilityStatus::loading;
    int build_time_ = 0;
    int count_ = 0;

public:
    SpecialAbility(int game_object_id_, Strategy* strategy, std::string game_object_name, int energy, int max_energy. int build_time):
        game_object_id_(game_object_id),
        game_object_name_(game_object_name),
        strategy_(strategy),
        energy_(energy * 10000),
        max_energy_(max_energy * 10000),
        build_time_(build_time)
        {

        }

    virtual ~SpecialAbility() { delete strategy_; }

    //ALWAYS call notify before doSpecialThings
    virtual void notify()
    {
        energy_ += energy_rate_;

        if(energy_ > max_energy_) { energy_ = max_energy_; }     //not more than max_energy

        if(strategy_->getEnergyNeed() <= energy_) { ability_status_ = AbilityStatus::available; }

        //DEBUD Output for checking energy level
        //std::cout<< "Energy: " << energy_/10000. << std::endl;
    }

    //type aliases in header subscriber
    virtual Event doSpecialThings(GameObjMap& game_objects, EventQueue& event_list, int glob_time, int events_occured)
    {
        //counts time till building actually finished and keeping energy on that level
        if(count_ < build_time_)
        {
            ++count_;
            energy_ -= energy_rate_;
            Event e;
            return e;
        }

        if(count_ == build_time_)
        {
            count_ = build_time_ + 1;
            energy_ -= energy_rate_;
        }

        if(events_occured > 0)
        {
            Event e;
            return e;
        }

        if(ability_status_ != AbilityStatus::available || strategy_->getEnergyNeeded() > energy_)
        {
            Event e;
            return e; //return the dummy
        }

        /*
        - here, the actual special ability has been triggered:
        - we use our strategy to check if we do something or not
        - for decisions, game_objects and events can be used
        - also, we can set the status of a building to boosted
        - if strategy decides not to boost, we get a dummy Event back and hand it over to race control which can check if something was done
        - (and set eventbuffer etc)
        */

        //returns dummy (default Event) if nothing was done
        Event e = strategy_->triggerSpecialAbility(game_objects, events, glob_time, game_object_id_);

        if(e.getEndTime() != -1) //no dummy, pay energy
        {
            energy_ -= strategy_->getEnergyNeeded();

            //new ability will get set the next time notify() is called
        }

        return e;
    }

    int getGameObjectID() { return game_object_id_; }
    void setGameObjectID(const int id) { game_object_id_ = id; }

    std::string getGameObjectName() { return game_object_name_; }
    void setGameObjectName(std::string name) { game_object_name_ = name; }

    AbilityStatus getAbilityStatus() { return ability_status_; }

    void setBuildTime(int i)
    {
        build_time_ = i;
        count_ = 0;
    }

};
