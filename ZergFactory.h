#pragma once

#include <fstream>
#include <list>
#include <algorithm>

#include "Factory.h"
#include "RaceControl.h"
#include "SpecialAbility.h"
#include "LarvaeSpecialAbility.h"

#define DEBUGZERG 0

class ZergFactory: public Factory
{
public:
    ZergFactory(const int workersID) : Factory(workersID) {}

};
