#pragma once

#include <string>
#include "WorkerControl.h"

class ZergWorkerControl : public WorkerControl
{
public:
    ZergWorkerControl(const int workers): WorkerControl(workers, "drone")
    { }

    void killWorker()
    {
        if(status_idle_ > 0) { --status_idle_; }

        else if(status_mineral_ > 0) { --status_mineral_; }

        else if(status_gas_ > 0) { --status_gas_; }

        else { throw new std::string("Zerg WC: tried to delete worker but none exists"); }
    }

    void freeWorker() { return; }

};
