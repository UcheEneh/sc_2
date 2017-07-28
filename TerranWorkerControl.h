#pragma once

#include "WorkerControl.h"

class TerranWorkerControl : public WorkerControl
{
    int mule_amount_ = 0;

public:
    TerranWorkerControl(const int workers) : WorkerControl(workers, "scv")
    {
        //for initialization, tell all workers to gather minerals
    }

    void addMULE()
    {
        ++mule_amount_;
        std::cout << "MULE ADDED. current: " << mule_amount_ << std::endl; //DEBUG
    }

    void removeMULE()
    {
        if(mule_amount_ > 0)
        {
            --mule_amount_;
            std::cout << "MULE REMOVED. current: " << mule_amount_ << std::endl; //DEBUG
        }
    }

    int getMULE() { return mule_amount_; }

    //These derived functions from WorkerControl are changed to be compatible with MULE
    std::pair<int, int> deliverResources()
    {
        std::pair<int, int> resources (rate_mineral_ * (status_mineral_ + mule_amount_ * 4), rate_gas_ * status_gas_);
        return resources;
    }
};
