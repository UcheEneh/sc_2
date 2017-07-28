#pragma once

#include "Factory.h"

class ProtossFactory : public Factory
{
public:
    ProtossFactory(const int workersID): Factory(workersID)
    {
        int i = 0;
        ++i;
        ++i;
    }

    bool validateBuildList(std::list<std::string> list) const
    {
        return true;

        std::ofstream log;
        log.open("valbl_log.txt");
        if(log.fail())
        {
            std::cout << "Error: opening valbl_log.txt failed" << std::endl;
            return false;
        }

        log << "Writing this to a file. \n";

        //only works if used before any simulation
        int workers_alive = this->global_id_ - 1;
        int supply = 10 - workers_alive; //10 supply provided by Nexus
        std::vector<std::string> build_successful;

        //flag to indicate if there was period where vespene was gathered (i.e assimilator has been built and at least one probe available)
        bool vespene_gathered = false;

        //add workers to build successful, also add basis building
        for(int i = 0; i<workers_alive; ++i) { build_successful.push_back("probe"); }
        build_successful.push_back("nexus");

        //print intial build to log
        log << "initial build \n\n";

        for(std::string obj : build_successful) { log << obj << "\n"; }

        log << "initial supply: \t\t" << supply << "\n";
        log << "initial number workers: \t\t" << workers_alive << "\n\n";

        //iterate over all commands, each time check if prerequisites are met
        for(auto it = list.cbegin(); it != list.cend(); ++it)
        {
            log << "\t\t >>BUILDING<<\n" << "build " << *it << "\n\n";

            //empty command indicates (if) last line of file
            if(it->compare("") == 00)
            {
                log << "found empty line (assuming buildlist is finished) \n";
                break;
            }

            //lookup command in tech_tree_, if not found, something went wrong
            auto curr_entry_it = tech_tree_.find(*it);
            if(curr_entry_it == tech_tree_.cend())
            {
                log << "INVALID CIMMAND \n could not find given command in tech_tree_ \n" << "\t\t >>BUILDLIST INVALID <<"
                << std::endl;
                log.close();
                return false;
            }

            /*
            - at this point, given command considered calid
            - check dependency, producers and vespene requirement
            - also only two assimilators per base building
            */
            //Testing: check base_building and assimilator
            if(it->compare("assimilator") == 0)
            {
                int count_base_building = 0;
                int count_extractors = 0; //for assimilator

                for(std::string k : build_successful)
                {
                    if(k.compare("assimilator") == 0) //count refinery
                        { ++count_extractors; }
                    else if(k.compare("nexus") == 0) //count base buildings
                    { ++count_base_building; }
                }

                if(count_extractors + 1 <= 2*count_base_building) //valid
                {
                    log << "trying to build assimilator \n already owing" << count_extractors <<
                    "assimilator \t\t found" << count_base_building << " base buildings \n" <<
                    "\nassimilator can be built" << std::endl;
                }
                else //INVALID
                {
                    log << "INVALID COMMAND \n assimilator can't be built \n" << "tried for " <<
                    count_extractors + 1 << "assimilator, but only found " << count_base_building <<
                    "base buildings\n" << "\t\t >>BUILDLIST INVALID<<" <<std::endl;

                    log.close();
                    return false;
                }
            }

            std::vector<std::string> producers = curr_entry_it->second.getProducers();

            //if unit costs vespene but there's no assimilator
            if(curr_entry_it->second.getvespeneCost() > 0 && !vespene_gathered)
            {
                log << "vespene required! \n for building " << curr_entry_it->second.getvespeneCost() <<
                " vespene is required but for now, it has not been gathered\n";

                log << "\t\t >>BUILDLIST INVALID<<" << std::endl;
                log.close();
                return false;
            }


            //Testing: check for producers
            if(std::find(producers.cbegin(), producers.cend(), "probe") != producers.cend())
            {
                if(workers_alive > 0)
                {
                    build_successful.push_back(*it);
                    supply += curr_entry_it->second.getSupplyProvide();
                    supply -= curr_entry_it->second.getSupplyCost();

                    log << "build requirement was probe, build requirement met \n\n" <<
                    "\t\t>>BUILD SUCCESSFUL<<\n" << "new supply: " << supply << "\n" <<
                    "new workers alive: " << workers_alive << "\n\n";
                }
                else
                {
                    log << "no probes available, then something went wrong\n" <<
                    "\t\t>>BUILDLIST INVALID<<\n" << "workers alive: " << workers_alive <<
                    "\n\n";
                }
            }
        }
    }
};
