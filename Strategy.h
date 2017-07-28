#pragma once

#include <iostream>
#include <string>

#include <list>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>

#include "Event.h"
#include "GameObject.h"
#include "PrintStatus.h"

//type aliases
using GameObjMap = std::unordered_map<std::string, std::pair<std::vector<GameObject>, int> >;

using EventQueue = std::priority_queue<Event, std::vector<Event>, std::greater<Event> >;

class Strategy
{
protected:
    const int energy_needed_ = 0;

public:
    Strategy() {}
    virtual ~Strategy() {}

    // Here, the strategy for nexus orbital_command etc have to be implemented
    // derive subclass from strategy and implement

    virtual Event triggerSpecialAbility (GameObjMap& game_objects, EventQueue& events, int glob_time, int id) = 0;  //makes this class a (pure or abstract) base class

    virtual int getEnergyNeeded() = 0;

    virtual Strategy* clone() = 0; //clone used to make the exact copy of the object

};


class NexusStrategy : public Strategy
{
    Event triggerSpecialAbility(GameObjMap& game_objects, EventQueue& events, int glob_time, int id)
    {
        //if nothing is triggered, return dummy
        //implement if chronoboost should be executed or not
        // chronoboost speeds up production rate of specific buildings by 50% for 20 seconds
        //"iterate" over events -> no iterator possible

        Event e;
        return e;
    }

    Strategy* clone() { return new NexusStrategy(); }

    int getEnergyNeeded() { return energy_needed_; }

protected:
    const int energy_needed_ = 25 * 10000;
};


class OrbitalStrategy : public Strategyid
{
    Event triggerSpecialAbility(GameObjMap& game_objects, EventQueue& events, int glob_time, int id)
    {
        //if nothing is triggered return dummy
        // Implement MULE: MULE consumes 50 energy and creates temporary unit Mobile Utility Lunar Excavator
        // somehow we have to hand this new worker to the workerControl maybe via the created event?

        // needs no supply, dies after 90s, does not conflict with already harvesting scvs
        //4 times the harvesting rate of an SCV

        //-> Implement for optimization task
        //Extra supply takes 4s to become active. One time increase to supply depot of 8. Only availablide once per supply depot

        std::cout << "ACTIVATED OBITAL STRATRGY" << std::endl;

        std::list<GameObject> a; //dummy!!!!!!!!!!!!!!!!!id
        GameObject b; //dummy!!!!!!!!!111

        Event e(a, PrintStatus::specialAbility, b, glob_time + 90);
        std::list<int> l;
        l.push_back(id);
        e.setName("mule");
        e.setIDs(l);

        return e;
    }

    Strategy* clone() { return new OrbitalStrategy(); }

    int getEnergyNeeded() { return energy_needed_; }

protected:
    const int energy_needed_;
};


class QueenStrategy : public Strategy
{
    //at cost of 25 energy, queen can place 4 eggs into hatchery, /after 40s eggs become larvae
    //hatchery/lair/hive stops spawning larvae when it has >= 3. It will start when larva count < 3

    Event triggerSpecialAbility(GameObjMap& game_objects, EventQueue& events, int glob_time, int id)
    {
        // at this point, no larva event was triggered for the current time step
        // wait till queen is built

        //whenever possible, inject larva into the building with lowest amount of larva

        //-> Optimization: use larvae for building
        //here (now), larva taken at random

        std::vector<std::string> injectable_buildings_;
        injectable_buildings_.push_back("hatchery");
        injectable_buildings_.push_back("lair");
        injectable_buildings_.push_back("hive");

        std::vector<GameObject>& larvae_vec = game_objects.at("larva").first;

        //will contain the Hatchery/Lair/Hive with lowest larvae count
        GameObject lowest_larvae_count;
        int lowest_sum = 20;    //this is a dummy value and not possible

        assert (game_objects.at("hatchery").first.size() + game_objects.at("lair").first.size() + game_objects.at("hive").first.size() > 0);

        //loop over all possible injection targets
        for (std::string building : injectable_buildings_)
        {
            std::vector<GameObject>& buildings_vec = game_objects.at(building).first;

            for(GameObject obj : buildings_vec)
            {
                //skip currently building hatcheries? TODO
                if(obj.getStatus() == BuildStatus::creating) { continue }

                int sum = 0;

                //check if there's no ongoing larva event at a time
                //check if building has been injected. Since larva creation is an Event, the larva currently "injected" will be visible through the BuildStatus::creating
                //count the larva currently at this building
                bool boosted = false;

                for (GameObject l : larvae_vec)
                {
                    if(l.getID() == obj.getID())
                    {
                        if(l.getStatus() == BuildStatus::creating) { boosted = true; }
                        ++sum;
                    }
                }

                //if we are at a currently injected building, move to the next one
                //if all buildings given already have been boosted, nothing will ever be assigned to the lowest_larvae_count and it will stay as dummy -> we can check for it
                if (boosted) { continue; }

                //check if we found a better target
                if (sum < lowest_sum)
                {
                    lowest_sum = sum;
                    lowest_larvae_count = obj;
                }
            }
        }
        //now lowest_larvae_level is found or still the dummy
        //check dummy
        if(lowest_larvae_count.getID() == -1 || lowest_sum = 19)
        {
            Event e;
            return e;
        }

        //if we actually found something: time to create some larvae and also an Event freeing them after 40s
        //create larvae: up to 4 but not more than 19 available at a building at once
        std::list<GameObject> larvae_list;

        for(int i =0; i<std::min(19 - lowest_sum, 4); ++i)
        {
            //larva are their own producers
            //this approach sets BuildStatus to idle_collectingRes of the larva twice: in freeProducer() and in freeObject() of Event
            GameObject go("larva", lowest_larvae_count.getID(), BuildStatus::creating);
            larvae_list.push_back(go);
            game_objects.at("larva").first.push_back(go);
            ++game_objects.at("larva").second;
        }

        Event e(larvae_list, PrintStatus::specialAbility, lowest_larvae_count, glob_time + 40);
        e.setProducerID(id);
        return e;
    }

    Strategy* clone() { return new QueenStrategy(); }

    int getEnergyNeeded() { return energy_needed_; }

protected:
    const int energy_needed_ = 25 * 10000;
};


class LarvaeStrategy : public Strategy
{
    Event triggerSpecialAbility(GameObjMap& game_objects, EventQueue& events, int glob_time, int id)
    {
        //this does nothing for larva, the larva is created int LarvaAbility, and also, they are added to the game_objects

        //add a larva to the game_objects pool. Larvae will have the id of the game object they belong to (game_object_id) and the name larva (for lookup)
        //this ability is always triggered when possible: automatic triggering
        Event e;
        return e;
    }

    Strategy* clone() { return new LarvaeStrategy(); }

    int getEnergyNeeded() { return energy_needed_; }

protected:
    const int energy_needed_ = 84375; //10000 * energy_rate * 15 :every 15 secs this will be ready
};


//This class does nothing. It's here for testing reasons
class DummyStrategy : public Strategy
{
    Event triggerSpecialAbility(GameObjMap& game_objects, EventQueue& events, int glob_time, int id)
    {
        Event e;
        return e;
    }

    Strategy* clone() { return new DummyStrategy(); }

    int getEnergyNeeded() { return energy_needed_; }

protected:
    const int energy_needed_ = -1;
};

