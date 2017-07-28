#pragma once

#include <iostream>
#include <string>

#include <unordered_map>
#include <queue>
#include <vector>
#include <list>

#include <cassert>
#include <utility>

#include "Publisher.h"
#include "Event.h"
#include "GameObject.h"
#include "WorkerControl.h"
#include "Factory.h"
#include "SpecialAbility.h"
#include "AbilityStatus.h"

//TODO report worker reassignment if no events occured
//TODO let workers gather vespene if build method fails because some other building is not ready yet

class RaceControl : public publisher
{
protected:
    //make constructor protected so it can't be called with stuff like pointers, wc, fc ...
    //wc, fc always come from constructors in Terran/Protoss/Zerg RaceControl
    RaceControl(const int workers, const int vespene_gas, const int minerals, const int supply, const std::string tech_tree_path, const std::string racename,
                WorkerControl* wc, Factory* fa) : time(0), minerals_(minerals), vespene_gas_(vespene_gas), supply_(supply), supply_used_(0)
                //supply is set in constructor of inheriting class where workers and other stuff are added
                {
                    wc_ = wc;
                    fa_ = fa;

                    if(!(fa_->readTechtree(tech_tree_path, racename)))
                    { throw new std::string("error!"); }

                    //initialize all lists int game_objects_
                    std::vector<std::string> names = fa_->getObjectNames();

                    for(auto it - names.cbegin(); it != names.cend(); ++it)
                    {
                        std::vector<GameObject> l;
                        game_objects_.insert({*it, std::make_pair(l,0)});
                        //DEBUG std::cout << "List created for: " << *it << std::endl;
                    }

                    //construct workers in race's own factory
                    //don't forget to add base building in own RC class
                }

public:
    //no destructor in derived classes necessary as this is called automatically
    //except you introduce new variables that need memory handling
    virtual ~RaceControl()
    {
        delete wc_;
        delete fa;
        //hopefully prevents memory leaks
        for(SpecialAbility* ptr : observer_list_)
        { delete ptr; }
    }

    //prints a message id something happened
    void printLog(std::ostream& stream, const int n_tabs)
    {
		/* for debugging */
		if(buffer_events_.empty()) { return; }
		static bool first = true;

		if(!first) { stream << ",\n" << std::flush; }

		else { first = false; }

		for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "{\n";

		for(int i =0; i<n_tabs + 2; ++i) { stream << "\t"; }
		stream << "\"time\": " << time_ << ",\n";

		printStatus(stream, n_tabs + 2);
		stream << ",\n";

		printEvents(stream, n_tabs + 2);
		stream << "\n";

		for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "}";
    }

    void printInitialBuild(std::ostream& stream, int workers, std::string race, int n_tabs)
    {
        for(int i =0; i<n_tabs; ++i) { stream << "\t"; }
		stream << "\"initialUnits\": {\n";
		bool first_list = true;

		for(auto e : game_objects_)
        {
            if(!e.second.first.empty())
            {
                if(first_list) { first_list = false; }
                else { stream << ",\n"; }

                for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
                stream << "\"" << e.first << "\": [\n";

                bool first = true;
                for (GameObject go : e.second.first)
                {
                    if(!first) { stream << ",\n"; }

                    else { first = false; }

                    for(int i =0; i<n_tabs + 2; ++i) { stream << "\t"; }
                    stream << "\"" << go.getName() << "_" << go.getID() << "\"";
                }
                stream << "\n";

                for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
                stream << "]";
            }
        }
        stream << "\n";

        for(int i =0; i<n_tabs; ++i) { stream << "\t"; }
		stream << "},\n";
    }

protected:
    void printStatus(std::ostream& stream, const int n_tabs)
    {
        for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "\"status\": {\n";

		printJSONMinerals(stream, n_tabs + 1);
		stream << ",\n";

		wc_->printJSON(stream, n_tabs + 1);
		stream << "\n";

		for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "}";
    }

    void printEvents(std::ostream& stream, const int n_tabs)
    {
        for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "\"events\": [\n";

		//flag for commas
		bool first = true;

		//print and pop all events in buffer_events_
		while(!buffer_events_.empty())
        {
            if(buffer_events_.front().getPrintStatus() == PrintStatus::noprint)
            {
                buffer_events_.pop_front();
                continue;
            }
            if(!first) { stream << ",\n"; }

            //print events from this timestep only
            if(first) { first = false; }

            (buffer_events_.front()).printJSON(stream, n_tabs + 1);
            buffer_events_.pop_front();
        }
        stream << "\n";

        for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "]";
    }

    void printJSONMinerals(std::ostream& stream, const int n_tabs) const
    {
        for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "\"resources\": {\n";

		for(int i =0; i<n_tabs + 2; ++i) { stream << "\t"; }
		stream << "\"minerals\": " << minerals_/10000. << ",\n";

		for(int i =0; i<n_tabs + 2; ++i) { stream << "\t"; }
		stream << "\"vespene\": " << vespene_gas_/10000. << ",\n";

		for(int i =0; i<n_tabs + 2; ++i) { stream << "\t"; }
		stream << "\"supply-used\": " << supply_used_ << ",\n";

		for(int i =0; i<n_tabs + 2; ++i) { stream << "\t"; }
		stream << "\"supply\": " << supply_ << "\n";

		for(int i =0; i<n_tabs + 1; ++i) { stream << "\t"; }
		stream << "}";
    }

public:
    bool validateBuildlist(const std::list<std::string> build_list) const
    { return fa_->validateBuildList(build_list); }

    bool build(const std::string name, const int glob_time)
    {
        //optimizeGathering now returning true/false -> depending on if worker distribution has changed or not
        //new behaviour:
        bool done = false;

        if(minerals_ >= fa_->getCosts(name).first && vespene_gas_ >= fa_->getCosts(name).second)
        { done = wc_->optimizeGathering(0,0); }

        else
        { done = wc_->optimizeGathering(fa_->getCosts(name).first, fa_->getCosts(name).second); }

        if(done)
        {
            Event e;
            e.setPrintStatus(PrintStatus::noprint);
            buffer_events_.push_back(e);
        }

        if(name.compare(vespene_building_) == 0)
        {
            if(2*basis_built <= vespene_built) { return false; }
        }

        //lookup dependency
        const std::vector<std::string>& deps(fa_->getDependencies(name));
        bool found = false;

        if(deps.size() > 0 && deps[0].compare("") != 0)
        {
            for(std::string d : deps)
            {
                if((game_objects_.find(d)->second.first.size() - game_objects_.find(d)->second.second.size()) > 0)
                { found = true; }
            }

            if(!found)
            {
                std::endl;
                return false;
            }
        }

        //lookup producer
        const std::vector<std::string>& prods(fa_->getProducers(name));
        GameObject* producer = nullptr;
        found = false;

        for(std::string p : prods)
        {
            std::vector<GameObject>& producer_list = game_objects_.find(p)->second.first;

            if(producer_list.size() != 0)
            {
                for(unsigned int i = 0; i<producer_list.size(); ++i)
                {
                    if(producer_list.at(i).getStatus() == BuildStatus::idle_collectingRes)
                    {
                        producer = &(producer_list.at(i));
                        found = true;
                        break;
                    }
                }

                if(found) { break; }
            }
        }

        if(found == false)
        {
            //std::cout << "producer not found for: " << name << std::endl;
            return false;
        }

        time_ = glob_time;

        /*
        - call Factory with name of GameObject to be built
        - pass reference to "this" to Factory
        - if we cant pay min/gas/supply, dummy is returned

        - factory will register a new SpecialAbility in observer_list_ if name is updatable
        */
        std::list<GameObject> go_list = (fa_->build(name, minerals_, vespene_gas_, supply_, supply_used_, observer_list_));

        if(go_list.front().getName().compare(vespene_building_) == 0)
            { ++vespene_built; }

        for (GameObject& go_ref : go_list)
        {
            //check for dummy (i.e could not pay costs)!!!!!!!!!
            if(go_ref.getID() = -1)
            {
                //set workers in WorkerControl
                return false;
            }
            //add to collection
            //increase counter in gameobjects list
            ((game_objects_.at(name)).first).push_back(go_ref);
            ++game_objects_.at(name).second;

            /*
            - create Event with calculated time when building process is finished
            - add Event to eventlist
            */
        }
        // -Zerg and Protoss overload this
        occupyProdWork(producer, go_list.front());
        Event build_event(go_list, PrintStatus::buildEventStart, *producer, (time_ + fa_->getBuildTime(name)));

        //push is the actual insert operation
        event_list_.push(build_event);

        //Now also add event to buffer_events for printing
        buffer_events_.push_back(build_event);

        return true;
    }

    /*
    - if we take the specialAbility approach, we need to check if a unit morphing once had a special ability and then remove that one too
    - also, we have to edit out notify observers
    */
    virtual void update(int glob_time)
    {
        time_ = glob_time; //needed for end game
        std::pair<int, int> delivered_resources(wc_->deliverResources()); //get resources. first = mineral, second = gas
        addMinerals(delivered_resources.first);
        addVespene(delivered_resources.second);

        if(!event_list_.empty())
        {
            //pop all front finished events
            Event e;

            //check if time is met
            while(!event_list_.empty() && (e = event_list_.top()).getEndTime() == time_) //remember priority queue
            {
                //pop finished events
                event_list_.pop();

                //changing status (for printing)
                if(e.getPrintStatus() == PrintStatus::buildEventStart)
                {
                    GameObject p = e.freeProducer(game_objects_);
                    GameObject o = e.freeObject(game_objects_);

                    //telling workercontrol that we built another vespene building
                    if(o.getName().compare("command_center") == 0 || o.getName().compare("hatchery") == 0 || o.getName().compare("nexus") == 0)
                        { ++basis_built; }

                    if(o.getName().compare(vespene_building_) == 0 )
                        { wc_->addVespeneBuilding(); }

                    if(wc_->isWorker(p)) //if producer is a Worker, set him free
                        { wc_->freeWorker(); } //also optimize worker distribution here

                    if(fa_->getMorph(o.getName())) //erase morphed worker
                    {
                        std::vector<GameObject>& vg = game_objects_.at(p.getName()).first;
                        bool flag_erased = false;

                        for(auto it = vg.begin(); it != vg.end; ++it)
                        {
                            if(p.getID() == it->getID())
                            {
                                vg.erase(it);
                                flag_erased = true;
                                break;
                            }
                        }
                        //zerg only
                        transferLarvae(p, o);

                        assert(flag_erased);
                    }

                    if(wc_->isWorker(o)) //if produced object is a worker, set it idle
                        { wc_->addWorker(); } //also optimize worker distribution here

                    supply_ += fa_->getSupplyProvide(o.getName()); //add provided supply if there is sth > 0
                    e.setPrintStatus(PrintStatus::buildEventEnd);
                }

                else if(e.getPrintStatus == PrintStatus::specialAbility) //dont print end of specialAbility
                {
                    e.setPrintStatus(PrintStatus::noprint);

                    //freeing producer for larvae events
                    e.freeObject(game_objects_);
                }

                //store finished events since last print in temporaryEventLog, print it when finished
                //for now, we don't add noprint events
                if(e.getPrintStatus() != PrintStatus::noprint) { buffer_events_.push_back(e); }
            }
        }
        /*
        - since actual GameObjects are stored somewhere else, observer_list_ has to be a list of pointers, pointing to the actual GameObjects.
          This requires additional deletion in the destructor.
        - we are already using a list of pointers, in this case, we simply add stuff to the destructor

        - objects now get a reference of event_list_ and game_objects_ in order to give the ability to create events and for example boost stuff
        */
    }

    virtual void transferLarvae(GameObject& producer, GameObject& go) { }

    //needed?
    void addMinerals(const int amount) { minerals_ += amount; }
    void addVespene(const int amount) { vespene_gas_ += amount; }

    bool eventsFinished()
    {
        std::list<Event> ll;
        while(event_list_.empty() == false)
        {
            ll,push_back(event_list_.top());
            event_list_.pop();
        }
        int count = 0;

        for(Event e : ll)
        {
            if(e.getPrintStatus() != PrintStatus::specialAbility) { ++count; }
            event_list_.push(e);
        }

        if(count == 0) { return true; }

        return false;
    }

    void registerObserver (SpecialAbility* observer) { observer_list_.push_back(observer); }
    void unregisterObserver (SpecialAbility* observer) { return; }

public:
    virtual void notifyObservers()
    {
        for(SpecialAbility* sp : observer_list_) { sp->notify(); }

        int events_occured = 0;
        for(Event e : buffer_events_)
        {
            if(e.getPrintStatus() == PrintStatus::buildEventStart) { ++events_occured; }
        }

        for(SpecialAbility* sp : observer_list_)
        {
            Event e = sp->doSpecialThings(game_objects_, event_list_, time_, events_occured);
            if(e.getEndTime() != -1)
            {
                buffer_events_.push_back(e);
                event_list_.push(e);

                //after event has been successfully created, the order will be executed in this function, so we don;t have to cahnge the whole notifyObservers
                executeSpecialOrder();

                breal;
            }
        }
        return;
    }

protected:
    virtual void executeSpecialOrder() { }

    virtual void occupyProdWork(GameObject* producer, GameObject go)
    {
        producer->setStatus(BuildStatus::occupied);
        if(wc_->isWorker(*producer))
            { wc_->occupyWorker(); }
    }

    std::string vespene_building_ = ""; //race specific vespene gathering
    int time_; //current time

    //current inventory
    int minerals_;
    int vespene_gas_;
    int supply_;
    int supply_used_;

    //good style
    int basis_built = 1;
    int vespene_built = 0;

    //Simulation mechanics
    WorkerControl* wc_;
    Factory* fa_;

    //Event list
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> event_list_;

    //GameObject master list
    std::unordered_map<std::string, std::pair<std::vector<GameObject>, int>> game_objects_;

    //extract events stated this time step
    std::list<Event> buffer_events_;

};
