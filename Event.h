#pragma once

#include <string>
#include <iostream>
#include <list>

#include <unordered_map>
#include <vector>
#include <cassert>

#include "PrintStatus.h"
#include "GameObject.h"

class Event
{
public:
    Event() {}
    // ids is list because zerglings produce two at a time
    // Event(const std::string name, const std::list<int> ids, const int end_time,
    // const int producer_id = -1, PrintStatus status = PrintStatus::noprint, GameObject* producer):
    // name_ (name),
    // ids_ (ids),
    // end_time_ (end_time),
    // producer_id_ (producer_id),

    Event(std::list<GameObject>& obj_list, PrintStatus status, GameObject producer, const int end_time): // ":" for initializing member variables before the body of the constructor executes.
        name_(obj_list.front().getName()), //same as name_ = obj_list.front().getName()
        print_status_(status),
        end_time_(end_time),
        producer_name_(producer.getName()),
        producer_id_(producer.getID())
        {
            for(GameObject go : obj_list) //This runs through the contents of obj_list, with the variable "go" taking on the value of each element of the list, in series, until the end of
                                          //the list is reached. You can use auto in the type, to iterate over more complex data structures conveniently--for example, to iterate over a map
            {
                ids_.push_back(go.getID()) //pushes back all ID into ids_
            }
        }

    const std::string& getName() { return name_; }
    void setName(std::string name) { name_ = name; }

    const int getEndTime() { return end_time_; }

    GameObject& freeProducer(std::unordered_map<std::string, std::pair<std::vector<GameObject>, int> >& game_objects)
    {
        std::vector<GameObject>& vec = game_objects.at(producer_name_).first;

        for(unsigned int i=0; i<vec.size(); ++i)
        {
            if(vec.at(i).getID() == producer_id_)
            {
                vec.at(i).setStatus(BuildStatus::idle_collectingRes);
                return vec.at(i);
            }
        }

        throw new std::string("Producer could not be freed");
    }

    GameObject& freeObject(std::unordered_map<std::string, std::pair<std::vector<GameObject>, int> >& game_objects)
    {
        //for zerg, it is also sufficient to simply return one of the produced units
        // since they all have the same name. also only two zerglings can be built at once so this will never affect workers

        std::vector <GameObject>& vec = game_objects.at(name_).first;
        int found_first = -1;

        for (int id : ids_)
        {
            --game_objects.at(name_).second;
            assert(game_objects.at(name_).second >= 0);

            for (unsigned int i=0; i<vec.size(); ++i)
            {
                if(vec.at(i).getID()==id && vec.at(i).getStatus() != BuildStatus::idle_collectingRes)
                {
                    if(found_first == -1)
                        found_first = i;

                    vec.at(i).setStatus(BuildStatus::idle_collectingRes);
                    break;
                }
                //we should always be able to find that specific id
                if(i == vec.size() - 1)
                    throw new std::string("Object could not be freed");

            }
        }

        return vec.at(found_first);
//        Edit: should be Zerg accessible now
//        old stuff:
//        --game_objects.at(name_).second;
//        for(unsigned int i =0; i< vec.size(); ++i)
//        {
//            if(vec.at(i).getID() == ids_.front())
//            {
//                vec.at(i).setStatus(BuildStatus::idle_collectingRes);
//                return vec.at(i);
//            }
//        }
//        throw new std::string ("Object could not be freed");
    }

    std::list<int>& getIDs(){ return ids_; }
    void setIDs(std::list<int>& ids) { ids_ = ids}


    /* providing comparison operator for our priority_queue in RaceControl */
    bool operator>(const Event& e2)const { return this->end_time_ > e2.end_time_; }

    /* since there are special abilities that can possibly speed up build times, we should be able to modify the finishing time */
    void boostEndTime(int new_end_time)
    {
        end_time_ = new_end_time;
        //if the endtime is updated, there shall be a mechanism to update the priority queue of events as well
    }

    /* prints JSON format. Gets depth of indentation as n_tabs (number of tabs so far) */
    void printJSON(std::ostream& stream, const int n_tabs)
    {
        //super ugly switch case on print_status_
        for(int i=0; i < n_tabs + 1; ++i) { stream << "\t"; }

        if(print_status_ == PrintStatus::noprint) { return; }
        stream << "{\n";

        switch (print_status_)
        {
        case PrintStatus::noprint:
            break;

        case PrintStatus::specialAbility:
            this->printSpecialAbility(stream, n_tabs+1);
            break;

        case PrintStatus::buildEventStart:
            this->printBuildStart(stream, n_tabs+1);
            break;

        case PrintStatus::buildEventEnd:
            this->printBuildEnd(stream, n_tabs+1);
            break;

        default:
            stream << "ERROR: EVENT NOT FOUND";
        }

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "}";
    }

    void printLarvaeInjection(std::ostream& stream, const int n_tabs)
    {
        for (int i=0; i<n_tabs+1; ++i) { stream << "\t"; }
        stream << "\"type\": \"special\",\n";

        for (int i=0; i<n_tabs+1; ++i) { stream << "\t"; }
        stream << "\"name\": \"injectlarvae\",\n";

        for (int i=0; i<n_tabs+1; ++i) { stream << "\t"; }
        stream << "\"triggeredBy\": \"queen_" << producer_id_ << "\",\n";

        for (int i=0; i<n_tabs+1; ++i) { stream << "\t"; }
        stream << "\"targetBuilding\": \"" << producer_name_ << "_" << ids_.front() << "\"\n";
    }

    void printMule(std::ostream& stream, const int n_tabs)
    {
        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t" }
        stream << "\"type\": \"special\",\n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t" }
        stream << "\"name\": \"" << name_ << "\",\n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t" }
        stream << "\"triggeredBy\": \"" << "orbital_command_" << ids.front() << "\"\n";
    }

    void printSpecialAbility(std::ostream& stream, const int n_tabs)
    {
        if(name_.compare("larva") == 0)
        {
            printLarvaeInjection(stream, n_tabs);
            return;
        }

        if(name_.compare("mule") == 0)
        {
            printMule(stream, n_tabs);
            return;
        }

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"type\": \"special\",\n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"name\": \"" << name_ << "\"n";
    }

    void printBuildStart (std::ostream& stream, const int n_tabs)
    {
        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"type\": \"build-start\",\n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"name\": \"" << name_ << "\"n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"producerID\": \"" << producer_name_ << "_" << producer_id_ << "\"\n";
    }

    void printBuildEnd(std::ostream& stream, const int n_tabs)
    {
        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"type\": \"build-end\",\n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"name\": \"" << name_ << "\"n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"producerID\": \"" << producer_name_ << "_" << producer_id_ << "\",\n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "\"producedIDs\": [\n";

        bool first = true;

        for (int id : ids_)
        {
            if(!first) { stream << ",\n"; }

            else { first = false; }

            for (int i=0; i<n_tabs + 2; ++i) { stream << "\t"; } //////////////////// why + 2?
            stream << "\"name\": \"" << id << "\"";
        }
        stream << "\n";

        for (int i=0; i<n_tabs + 1; ++i) { stream << "\t"; }
        stream << "]\n";
    }

    PrintStatus getPrintStatus() { return print_status_; }
    void setPrintStatus (PrintStatus status) { print_status_ = status; }

    int getProducerID() { return producer_id_; }
    std::string getProducerName() { return producer_name_; }

    GameObject& getProducer (std::unordered_map<std::string, std::pair<std::vector<GameObject>, int> >& game_objects)
    {
        std::vector<GameObject>& vec = game_objects.at(producer_name_).first;

        for (unsigned int i = 0; i<vec.size(); ++i)
        {
            if(vec.at(i).getID() == producer_id_) { return vec.at(i); }
        }

        throw new std::string ("Producer could not be freed");
    }

    void setProducerID(int id) { producer_id_ = id; }
    void setProducerName(std::string name) { producer_name_ = name; }


private:
    std::string name_ = "";
    //int ids_;
    //formerly int ids_ is changed to list<int> ids_ since Zerg produces more than one unit at once for zergling
    std::list<int> ids_;
    PrintStatus print_status_;

    int end_time_ = -1;

    std::string producer_name_ = "";
    int producer_id_ = -1;

};
