#pragma once

#include <iostream>
#include <string>

#include <list>
#include <cassert>
#include <algorithm>

#include <utility>
#include <functional>

#include "GameObject.h"
#include "BuildStatus.h"

class WorkerControl
{
protected:
    //workers_stores references to the workers of RaceControl game_objects_
    int status_mineral_ = 0;
    int status_gas_ = 0;
    int status_building_ = 0;
    int status_idle_ = 0;
    std::string workername_ = "";
    int vespene_buildings_ = 0;

    //the rate is multiplied with 10000 compared to its real value (0.7 & 0.35)
    static const int rate_mineral_ = 7000;
    static const int rate_gas_ = 3500;

    //constructor is protected
    WorkerControl (int initialWorkers, std::string workername) : status_idle_(initialWorkers), workername_(workername)
    {
        status_gas_ = 0;
        status_mineral_ = 0;
        optimizeGathering();
    }

public:
    virtual void killWorker() { return; }

    //added because above method is virtual
    virtual ~WorkerControl() {}

    /* prints JSON format, gets depth of indentation as n_tabs (number of tabs so far) */
    void printJSON(std::ostream& stream, const int n_tabs) const
    {
        for (int i =0; i<n_tabs +1; ++i) { stream << "\t"; }
        stream << "\"workers\": {\n";

        for (int i =0; i<n_tabs +2; ++i) { stream << "\t"; }
        stream << "\"minerals\": " << status_mineral_ << ",\n";

        for (int i =0; i<n_tabs +2; ++i) { stream << "\t"; }
        stream << "\"vespene\": " << status_gas_ << "\n";

        for (int i =0; i<n_tabs +1; ++i) { stream << "\t"; }
        stream << "}" << std::flush;
    }

    void addWorker()
    {
        //just tell Worker control that one more worker is available
        ++status_idle_;

        optimizeGathering();

        //DEBUG std::cout << "Worker added: " << (workers_.back().get()).getID << std::endl;
    }

    //Resource-Pair of <Mineral,Gas>
    virtual std::pair<int, int> deliverResources()
    {
        std::pair <int,int> resources (rate_mineral_ * status_mineral_, rate_gas_ * status_gas_);
        return resources;
    }

    bool optimizeGathering(int minerals, int gas)
    {
        int cur_gas = status_gas_;
        int cur_idle = status_idle_;
        int cur_mineral = status_mineral_;
        int sum = status_gas_ + status_idle_ + status_mineral_;

        if(sum == 0) { return false; }

        //new behaviour for the case we don't actually need minerals/vespene
        if(minerals == 0 && gas == 0)
        {
            if(sum > vespene_buildings_ * 3)
            {
                status_gas_ = vespene_buildings_*3;
                sum -= status_gas_;
            }
            status_mineral_ = sum;
        }

        else
        {
            int min_time = minerals*10000/rate_mineral_;
            int gas_time = gas*10000/rate_gas_;

            int min_workers = min_time/(gas_time + min_time) * (sum);
            int gas_workers = sum - min_workers;

            int difference = gas_workers - vespene_buildings_*3;
            if(difference > 0)
            {
                gas_workers -= difference;
                min_workers += difference;
            }

            if(vespene_buildings_ != 0 && gas_workers == 0 && gas != 0)
            {
                gas_workers = 1;
                min_workers = std::min(0, gas_workers - 1);
            }

            status_mineral_ = min_workers;
            status_gas_ = gas_workers;
            status_idle_ = 0;
        }

        if (cur_gas != status_gas_ || cur_idle != status_idle_ || cur_mineral != status_mineral_) { return true; }

        return false;

        //std::cout << "Special Optimization: Gas: " << status_gas_ << " Idle: " << status_idle_ << " Minerals: " << status_mineral_ << std::endl;
    }

    void optimizeGathering()
    {
        while(status_idle_ > 0)
        {
            //If vespene is available && if less worker than possible && every time there are more workers for mineral than gas
            if(vespene_buildings_ > 0 && (vespene_buildings_*3) > status_gas_ && status_mineral_ > status_gas_)
            {
                ++status_gas_;
                --status_idle_;
            }
            else
            {
                ++status_mineral_;
                --status_idle_;
            }
        }
    }

    bool occupyWorker()
    {
        //someone is working
        assert(status_idle_ + status_gas_ + status_mineral_ > 0);

        if(status_idle_ > 0) //TODO compile dummy
        {
            ++status_building_;
            --status_idle_;
            return true;
        }
        else if(status_mineral_ > 0)
        {
            ++status_building_;
            --status_mineral_;
            return true;
        }
        else if(status_gas_ > 0)
        {
            ++status_building_;
            --status_gas_;
            return true;
        }
        else
        {
            throw new std::string ("There are no workers to occupy");
            return false;
        }
    }

    //overloaded by zerg -> there are no free workers
    virtual void freeWorker()
    {
        //if building event has been finished, i.e. free Worker
        --status_building_;
        ++status_idle_;

        optimizeGathering();
    }

    bool isWorker(GameObject& g) //check if object is a worker
    {
        if(g.getName().compare(workername_) == 0) { return true; }
        else {return false; }
    }

    void addVespeneBuilding() { ++vespene_buildings_; }

    //these are needed for Terran and MULEs
    virtual void addMULE() {}
    virtual void removeMULE() {}
    virtual void getMULE() { return 0; }
};
