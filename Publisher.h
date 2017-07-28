#pragma once

#include <vector>
#include "SpecialAbility.h"

class publisher
{
public:
    publisher() {}
    //append observer to observerList
    virtual void registerObserver(SpecialAbility* observer) = 0;

    //delete specific member of observerList
    virtual void unregisterObserver(SpecialAbility* observer) = 0;

    //notify each member of the observerList
    virtual void notifyObservers() = 0;

protected:
    //list should only store pointers to actual objects
    //objects themselves stored in gameObjects list

    //It is likely pointers will get invalidated!!!!!!!!!!!
    std::vector<SpecialAbility*> observer_list_;

};
