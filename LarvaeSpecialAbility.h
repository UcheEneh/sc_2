#pragma once
#include "SpecialAbility.h"
#include "Strategy.h"

#include <cassert>

//created when hatchery is created
//there should be a initial constructor for this if we have the initial larva building -> it starts with three LarvaeStrategy

class LarvaAbility : public SpecialAbility
{
public:
    LarvaAbility(int game_object_id, std::string game_object_name, int energy, int max_energy, int init_larvae = 0, int build_time = 0) :
        SpecialAbility(game_object_id, new LarvaeStrategy(), game_object_name, energy, max_energy, build_time),
        larvae_(int_larvae)
        {

        }

    //do events finish first> -> 4 larvae possible if events finish after notifying and triggering ability
    void notify()
    {
        /*
        - notifying it done in triggerSpecialAbility -> LarvaeStrategy is always triggerable since larvae are counted in doSpecialThings
          it might be that after a larva is created in a timestep it is used right after without decreasing the counter of larvae_ here -> each
          time a larva is used in a timestep it will take notify one timestep to notify this
        */
    }

    //type aliases in header subscriber
    virtual Event doSpecialThings(GameObjMap& game_objects, EventQuene& events, int glob_time, int events_occured)
    {
        if(count_ < build_time_)
        {
            ++count_;
            energy_ = 0;
            Event e;
            return e;
        }

        //count owed larvae
        std::vector<GameObject> larvae_list = game_objects.at("larva").first;
        int larvae = 0;

        for(GameObject larv : larvae_list)
        {
            if(larv.getID() == game_object_id_ && larv.getStatus() != BuildStatus::creating && larv.getStatus() != BuildStatus::occupied)
            { ++larvae; }
        }

        assert(0 <= larvae && larvae <= 19);
        larvae_ = larvae;

        if(larvae_ >= 3) { energy_ = 0; }

        else { energy_ += energy_rate_; }

        //injection by queen can lead to too many larvae
        /*
        - here the actual special ability can be triggered: we use our strategy to check if we do something or not, for decisions, game_objects
          and events can be used.
        - also, we can set the status of a building to boosted

        - if strategy decides not to boost, we get a dummy Event back and hand it over to race control which then checks if something was done
        */

        //returns dummy (default Event): always, since this is an automatic ability
        Event e = strategy_->triggerSpecialAbility(game_objects, events, glob_time, game_object_id_);

        //check if ability is triggered
        if(energy_ == strategy_->getEnergyNeeded())
        {
            energy_ = 0;

            //for associating larvae wth their owners, they have no own id_ but a copy of the id of their woner
            GameObject go("larva", game_object_id_, BuildStatus::idle_collectingRes, glob_time);

            game_objects.at("larva").first.push_back(go);
            //new ability status will get set next time notify is called

            ++larvae_;
        }

        return e;
    }

protected:
    int larvae_;
    AbilityStatus ability_status_ = AbilityStatus::available;
};
