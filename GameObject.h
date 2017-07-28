#pragma once

#include <string>
#include "BuildStatus.h"

class GameObject
{
public:
    GameObject(){}
    virtual ~GameObject(){}

    GameObject(std::string name, const int id, BuildStatus status, int creation_time_ = 0):
        name_(name),
        member_id_(id),
        status_(status)
        {
        }

        int getID(){ return member_id_; }
        void setID(const int id){ member_id_ = id; }

        BuildStatus getStatus(){ return status_; }
        void setStatus(BuildStatus status){ status_ = status; }

        std::string getName () { return name_; }

        bool isUpdatable(){ return updatable_; }
        void setUpdatable(){ updatable_ = true; }

        int getCreationTime (){ return creation_time_; }

protected:
    std::string name_ = "";
    int member_id_ = -1;
    BuildStatus status_ = BuildStatus::occupied;
    bool updatable_ = false;
    int creation_time_ = 0;

};
