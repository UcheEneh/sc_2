#pragma once

#include "RaceControl.h"
#include "TerranWorkerControl.h"
#include "TerranFactory.h"

class TerranRaceControl : public RaceControl
{
    public:
    TerranRaceControl(const int workers, const int vespene_gas, const int minerals, const int supply, const std::string tech_tree_path, const std::string racename) :
    RaceControl(workers, vespene_gas, minerals, supply, tech_tree_path, racename, new TerranWorkerControl(workers), new TerranFactory(0))
    {
        //Tell race control that specific vespene building is named "refinery"
        vespene_building_ = "refinery";

        for(int i = 1; i <= workers; ++i)
        {
            GameObject go = fa_->buildInitial("scv", supply_, supply_used_, observer_list_);
            go.setStatus(BuildStatus::idle_collectingRes);
            game_objects_.at("scv").first.push_back(go);
        }

        //main building
        GameObject mainBuilding = fa_->buildInitial("command_center", supply_, supply_used_, observer_list_);
        mainBuilding.setStatus(BuildStatus::idle_collectingRes);
        game_objects_.at("command_center").first.push_back(mainBuilding);

        assert(supply_ >= supply_used_);
    }

    void occupyProdWork(GameObject* producer, GameObject& go)
    {
        if((producer->getName()).std::string::find("_with_reactor") != std::string::npos)
        {
            //the given producer has an update "reactor" and so can handle two build orders (command) parallel
            auto it = reactor_buildings_.find(producer->getID());
            if(it != reactor_buildings_.end())
            {
                //if reactor is found, then there is already an order so status is changed to occupied and then there can't be more than two orders at once
                if(reactor_buildings_.at(producer->getID()) == 1) //one order
                {
                    reactor_buildings_.at(producer->getID()) == 2;
                    producer->setStatus(BuildStatus::occupied);
                }
            }
            else
            {
                reactor_buildings_.insert(std::pair<int, int>(producer->getID(), 1));
                //reactor is added to the list so it is known that it already has an order to fulfill
                //It had no order before that so the current order count is set to 1
                //The status is not occupied so another order can be given
            }
        }
        else
        {
            //the given producer is a regular unit
            producer->setStatus(BuildStatus::occupied);

            if(wc_->isWorker(*producer))
                wc_->occupyWorker();
        }
    }

    void update(int glob_time)
    {
        time_ = glob_time; //needed for end game
        std::pair<int, int>delivered_resources(wc_->deliverResources()); //get resources. first: mineral, second: gas
        addMinerals(delivered_resources.first);
        addVespene(delivered_resources.second);

        if(!event_list_.empty())
        {
            //pop all front finished events
            Event e;
            //check if time is met
            while(!event_list_.empty() && (e = event_list_.top()).getEndTime() == time_) //remember proiority queue!
            {
                event_list_.pop //pop finished events

                //change status for printing
                if(e.getPrintStatus() == PrintStatus::buildEventStart)
                {
                    GameObject p = e.getProducer(game_objects_);

                    auto it = reactor_buildings_.find(e.getProducerID());
                    if(it != reactor_buildings_.end())
                    {
                        //if it is a currently created reactor buyilding then decrease no of current orders because one has just finished
                        it->second = it->second - 1;

                        //No of orders is <2 so current Build status should be idle again
                        p = e.freeProducer(game_objects_);

                        if((it->second) == 0) //if this was the last order, the reactor should be deleted from the map
                            reactor_buildings_.erase(it);
                    }
                    else //this is not a reactor building
                        p = e.freeProducer(game_objects_);

                    GameObject o = e.freeObject(game_objects_);

                    //telling worker control that we built another vespene building
                    if(o.getName().compare("command_center"))
                        ++basis_built;

                    if(o.getName().compare(vespene_building_) == 0)
                        wc_->addVespeneBuilding();

                    if(wc_->isWorker(p)) //if producer is worker, set it free
                        wc_->freeWorker(); //also optimize worker distribution here

                    if(fa_->getMorph(o.getName()))
                    {
                        std::vector<GameObject>& vg = game_objects_.at(p.getName()).first;
                        bool flag_erased = false;

                        for(auto it = vg.begin(); it != vg.end(); ++it)
                        {
                            if(p.getID() == it->getID())
                            {
                                vg.erase(it);
                                flag_erased = true;
                                break;
                            }
                        }
                        assert(flag_erased);
                    }

                    if(wc_->isWorker(o)) //if producer object is a worker, set to idle
                        wc_->addWorker(); //also optimize worker distribution here

                    supply_ += fa_->getSupplyProvide(o.getName());

                    e.setPrintStatus(PrintStatus::buildEventEnd);
                }

                else if(e.getPrintStatus() == PrintStatus::specialAbility) //don't print end of special ability
                {
                    //remove MULE
                    e.setPrintStatus(PrintStatus::noprint);
                    std::cout << time_ << " "; //DEBUG
                    wc_->removeMULE();
                }

                //save finished events since last print in temporaryEventLog, print it when finished
                //for now, don't add noprint events
                if(e.getPrintStatus() != PrintStatus::noprint)
                    buffer_events_.push_back(e);
            }
        }
        //??
    }

    void executeSpecialOrder()
    {
        /**
        Here, Terran only uses one special ability so we don't have to decide what is called
        Terran only needs to know that something related to special ability had been called.
        So, activate MULE from here and tell WorkerControl to do its job
        **/
        //if(wc_->getMULE())    //aomething is wrong here
        {
            std::cout << time_ << " "; //DEBUG
            wc_->addMULE();
        }
    }

    private:
    std::map<int, int> reactor_buildings_; //map of reactor buildings identified by their ID and mapped to their current build orders (<=2)
};
