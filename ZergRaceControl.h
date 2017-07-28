#pragma once

#include "RaceControl.h"
#include "ZergWorkerControl.h"
#include "ZergFactory.h"

class ZergRaceControl : public RaceControl
{
public:
   ZergRaceControl(const int workers, const int vespene_gas, const int minerals, const int supply, const std::string tech_tree_path, const std::string racename):
       RaceControl(workers, vespene_gas, minerals, supply, tech_tree_path, racename, new ZergWorkerControl(workers), new ZergFactory(0))
       {
           vespene_building_ = "extractor";

           for (int i = 1; i<= workers; ++i)
           {
               GameObject go = fa_->buildInitial("drone", supply_, supply_used_, observer_list_);
               go.setStatus(BuildStatus::idle_collectingRes);
               game_objects_.at("drone").first.push_back(go);
           }

           //main building
           GameObject mainBuilding = fa_->buildInitial("hatchery", supply_, supply_used_, observer_list_);
           mainBuilding.setStatus(BuildStatus::idle_collectingRes);
           game_objects_.at("hatchery").first.push_back(mainBuilding);

           //three initial larvae at hatchery
           for(int i=0; i<3; ++i)
           {
               GameObject go("larva", mainBuilding.getID(), BuildStatus::idle_collectingRes);
               game_objects_.at("larva").first.push_back(go);
           }

           ++supply_used_; //initial hatchery doesn't give supply through killing drone

           //initial overlord
           GameObject supply_provider = fa_->buildInitial("overlord", supply_, supply_used_, observer_list_);
           supply_provider.setStatus(BuildStatus::idle_collectingRes);
           game_objects_.at("overlord").first.push_back(supply_provider);

           //The ID will start from 1, because hatchery has ID 0

           assert(supply_ >= supply_used_);
       }

       void occupyProdWork(GameObject* producer, GameObject& go)
       {
           producer->setStatus(BuildStatus::occupied);

           if(wc_->isWorker(*producer)) { wc_->killWorker(); }
       }

       void transferLarvae(GameObject& producer, GameObject& go)
       {
           //transfer larvae and specialAbility if necessary
           if((producer.getName().compare("hatchery") == 0 && go.getName().compare("lair") == 0) ||
              (producer.getName().compare("lair") == 0 && go.getName().compare("hive") == 0))
           {
                //transfer larvae
                //std::cerr << "found producer " << producer.getName() << std::endl;
                //std::cerr << "transferring larvae" << stdd::endl;

                for(auto it = game_objects_.at("larva").first.begin(); it != game_objects_.at("larva").first.end(); ++it)
                {
                    if(it->getID() == producer.getID() && it->getStatus() != BuildStatus::occupied)
                        { it->setID(go.getID()); }
                }

                //std::cerr << "searching for events" << std::endl;
                //transfer currently running special events
                std::queue<Event> temp;
                while(!event_list_.empty())
                {
                    Event e = event_list_.top();
                    event_list_.pop();

                    if(e.getPrintStatus() == PrintStatus::specialAbility && e.getProducerName().compare(producer.getName()) == 0 &&
                       e.getIDs().front() == producer.getID())
                    {
                        //std::cerr << "found ongoing larva injection event" << std::endl;
                        e.setProducerName(go.getName());
                        e.setProducerID(go.getID());
                        std::list<int> ids = e.getIDs();

                        for(auto it = ids.begin(); it != ids.end(); ++it)
                        {
                            assert(*it == producer.getID());
                            *it = go.getID();
                        }
                        e.setIDs(ids);
                    }
                    temp.push(e);
                }
                //std::cerr << "finished searching events" << std::endl;

                while(!temp.empty())
                {
                    event_list_.push(temp.front());
                    temp.pop();
                }

                //transfer special ability
                for(auto it = observer_list_.begin(); it != observer_list_.end(); ++it)
                {
                    if((*it)->getGameObjectID() == producer.getID() && (*it)->getGameObjectName() == producer.getName())
                    {
                        (*it)->setGameObjectID(go.getID());
                        (*it)->setGameObjectName(go.getName());
                        return;
                    }
                }
           }
       }

private:
    int larvae_;

};
