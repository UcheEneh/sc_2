#pragma once
#include "WorkerControl.h"

class ProtossWorkerContol : public WorkerControl
{
public:
    ProtossWorkerContol(const int workers): WorkerControl(workers, "probe")
    {
    }

    void freeWorker()
    {
        //since workers can't be occupied, they can't get freed either
    }
};
