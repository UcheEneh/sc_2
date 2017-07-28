#pragma once
#include "RaceControl.h"
#include "ProtossFactory.h"
#include "ProtossWorkerControl.h"

class ProtossRaceContol : public RaceControl
{
public:
    ProtossRaceContol(const int workers, const int vespene_gas, const int minerals, const int supply, const std::string tech_tree_path, const std::string racename) :
        RaceControl(workers, vespene_gas, mineral, supply, tech_tree_path, racename, new ProtossWorkerContol(workers), new ProtossFactory(0))
        {
            vespene_building_ = "assimilator";

            //The ID starts from 1 because nexus has ID 0
            for(int i = 1; i <= workers; ++i)
            {
                GameObject go = fa_->buildInitial("probe", supply_, supply_used_, observer_list_);
                go.setStatus(BuildStatus::idle_collectingRes);
                game_objects_.at("probe").first.push_back(go);
            }

            //main building
            GameObject mainBuilding = fa_->buildInitial("nexus", supply_, supply_used_, observer_list_);
            mainBuilding.setStatus(BuildStatus::idle_collectingRes);
            game_objects_.at("nexus").first.push_back(mainBuilding);

            assert(supply_ > supply_used_);
        }

protected:
    void occupyProdWork(GameObject* producer, GameObject go)
    {
        if(!(wc_->isWorker(*producer))) { producer->setStatus(BuildStatus::occupied); }
    }
};
