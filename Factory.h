#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <map>
#include <vector>
#include <list>

#include <utility>

#include "TechTreeEntry.h"
#include "GameObject.h"
#include "SpecialAbility.h"
#include "AbilityStatus.h"
#include "Strategy.h"

#include "RaceControl.h"

class Factory
{
protected:
    Factory (const int workersID): global_id_(workersID)
    { }

public:
    virtual bool validateBuildList(std::list<std::string> list) const = 0;
    virtual std::list<GameObject> build(const std::string name, int& minerals, int& vespene, int& supply, int& supply_used, std::vector<SpecialAbility*>& observer_list)
    {
        std::list<GameObject> objects;

        //look up costs of object via map - if too high dummy is returned
        //Get costs for minerals, vespene and supply; everything again * 10000
        int cost_minerals = (tech_tree_.at(name)).getMineralCost() * 10000;
        int cost_vespene = (tech_tree_.at(name)).getVespeneCost() * 10000;
        int cost_supply = (tech_tree_.at(name)).getSupplyCost() * 10000;

        if(minerals >= cost_minerals && vespene >= cost_vespene && (supply - supply_used) >= cost_supply)
        {
            ++global_id_;

            //Pay the debt:
            minerals -= cost_minerals;
            vespene -= cost_vespene;
            supply_used += cost_supply;

            GameObject go(name, global_id_, BuildStatus::creating);

            //if our unit is a special one -> register SpecialAbility in observer_list
            for(std::string s : updatables_)
            {
                if(s.compare(name) == 0)
                {
                    //set go as updatable
                    go.setUpdatable();

                    //new approach : create a SpecialAbility here
                    int energy = (tech_tree_.at(name)).getInitialEnergy();
                    int max_energy = (tech_tree_.at(name)).getMaxEnergy();
                    Strategy* strategy = nullptr; //for when we need to create a strategy object

                    if(name.compare("nexus") == 0)
                        strategy = new NexusStrategy();

                    else if (name.compare("orbital_command") == 0)
                        strategy = new OrbitalStrategy();

                    else    //since Zerg is overloading this method
                        strategy = new DummyStrategy

                    SpecialAbility* s = new SpecialAbility(global_id_, strategy, name, energy, max_energy, tech_tree_.at(name).getBuildTime());
                    observer_list.push_back(s);

                    /*
                    This new approach would add SpecialAbility object to the observer_list
                    -> these would be notified and could also decide for themselves if they want to get triggered
                    We have to hand build the observer_list or somehow be able to add observers pr let it know that RC inherits from publisher
                    For the first implementation, handling the observer_list seemed to be easier.
                    */
                }
            }

            objects.push_back(go);
            return objects;
        }

        else
        {
            objects.push_back(GameObject("dummy", -1, BuildStatus::idle_collectingRes));
            return objects;
        }
    }

    std::pait<int, int> getCosts(std::string name)
    {
        int cost_minerals = (tech_tree_.at(name)).getMineralCost() * 10000;
        int cost_vespene = (tech_tree_.at(name)).getVespeneCost() * 10000;
        return std::pair<int,int> (cost_minerals, cost_vespene);
    }

    const std::vector<std::string>& getDependencies(const std:: string name) { return tech_tree_.find(name)->second.getDependencies(); }
    const std::vector<std::string>& getProducers(const std:: string name) { return tech_tree_.find(name)->second.getProducers(); }

    int getBuildTime(const std:: string name) { return tech_tree_.find(name)->second.getBuildTime(); }
    int getSupplyProvide(const std:: string name) { return tech_tree_.find(name)->second.getSupplyProvide(); }
    int getSupplyCost(const std:: string name) { return tech_tree_.find(name)->second.getSupplyCost(); }

    bool getMorph(const std:: string name) { return tech_tree_.find(name)->second.getMorph(); }

    virtual ~Factory() {}

    bool readTechtree(const std::string path, const std::string race)
    {
        //read tree to map
        /*
        TechTreeEntry can read its attributes fro, cvs formatted string scroll according to race
        Constructor in TechTreeEntry.h needed that takes a vector of strings and parses it into the object
        We iterate over all relevant lines and create objects from those lines stored in a vector
        */

        std::ifstream infile(path);
        std::string line;
        std::string token;

        if(infile.fail())
        {
            std::cout << "error: techtree could not be opened" << std::endl;
            return false;
        }

        do
        {
            std::getline(infile, line);
            //DEBUG std::cout << line << std::endl;
        }while(line.compare(0, 4 + race.size(), "# " + race + " #") != 0);   //here, race has to start with a capital

        //the ########### line
        std::getline(infile, line);

        //first line in relevant part
        std::getline(infile, line);

        //reading the units
        while(line.compare(0,1,"#") != 0) //loop over all actual lines
        {
            std::vector<std::string> line_vec;
            std::stringstream line_stream(line);

            while(std::getline(line_stream, token, ','))    //loop oversingle line splitting into tokenlist
            { line_vec.push_back(token); }

            //check for a trailing comma with no data after it.
            if(!line_stream && token.empty())
            {
                //if there was a trailing comma, then add an empty element
                line_vec.push_back("");
            }

            TechTreeEntry tte(line_vec);
            tech_tree_.insert(std::pair<std::string, TechTreeEntry> (line_vec[0], tte));
            std::getline(infile, line);
        }

        //skipping the #buildings,,,,,,, line
        while(std::getline(infile, line) && line.compare(0,1,"#") != 0)
        {
            std::vector<std::string> line_vec;
            std::stringstream line_stream(line);

            while(std::getline(line_stream, token, ','))  //loop over single line splitting it into token list
            { line_vec.push_back(token};

            //check for a trailing data with no comma after it
            if (!line_stream && token.empty())
            {
                //if there was a trailing comma then add an empty element
                line_vec.push_back("");
            }

            TechTreeEntry tte(line_vec);
            tech_tree_.insert(std::pair<std::string, TechTreeEntry> (line_vec[0], tte));
        }

        //check if all line reading has gone right
        if(infile.fail())
        {
            std::cout << "Error: something went wrong while reading in techtree" << std::endl;
            return false;
        }

        //close the stream after usage
        infile.close();
        if(infile.fail())
        {
            std::cout << "Error: after reading, techtree could not be closed" << std::endl;
            return false;
        }

        for (std::pair<std::string, TechTreeEntry> tte_pair : tech_tree_)
        {
            if(tte_pair.second.getMaxEnergy() != 0)
            { updatables_.push_back(tte_pair.second.getName()); }
        }

        return true;
    }


    void printTechtree()
    {
        for (auto it = tech_tree_.cbegin(); it != tech_tree_.cend; ++it)
        {
            std::cout << "key: " << it->first << ";\n\t" << "entry: " << it->second << "\n";
        }
        std::cout << std::endl;
    }

    /*
    This method returns all the object names stored in factories tech_tree_ called from constructor RaceControl in order
    to initialize the game_objects_ -> all lists in game_objects_ have to get initialized
    */
    std::vector<std::string> getObjectNames() const
    {
        std::vector<std::string> name_objects;
        for(auto it = tech_tree_.cbegin(); it != tech_tree_.cend; ++it)
            names_objects.push_back(it->second.getName());

        return names_objects;
    }

    /*
    - this method is added so that initial units will also get easily registered as observers
    - buildInitial builds units for free by invoking the normal behaviour method with costs as inventory
    - one cannot build morphed units here -> this would lead to wrong supply/supply_used!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */
    virtual GameObject buildInitial(std::string name, int& supply, int& supply_used, std::vector<SpecialAbility*>& observer_list)
    {
        //Get costs for minerals, vespene and supply; everything again * 10000
        int cost_minerals = (tech_tree_.at(name)).getMineralCost() * 10000;
        int cost_vespene = (tech_tree_.at(name)).getVespeneCost() * 10000;
        int cost_supply = (tech_tree_.at(name)).getSupplyCost() * 10000;
        int supply_provide = (tech_tree_.at(name)).getSupplyProvide() * 10000;

        supply += supply_provide;
        supply_used += cost_supply;

        int dummy = 0;;
        GameObject go = build(name, cost_minerals, cost_vespene, cost_supply, dummy, observer_list).front();
        return go;
    }

protected:
    //map with techtree content
    std::map<std::string, TechTreeEntry> tech_tree_;
    std::vector<std::string> updatables_;
    int global_id_;
};
