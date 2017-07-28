#pragma once
#include "Factory.h"

class TerranFactory : public Factory
{
public:
    TerranFactory(const int workersID): Factory(workersID)
    {}

    bool validateBuildList(std::list<std::string> list) const
    {
        //check for vespene building, not more than two per base
        /*
        Terran validate method:
        - check if vespene collected
        - check if not all workers morphed
        - atmost 2 vesp buiuldings per base

        initial build:
        - command_center(11 supply), id = 0
        - 6 scv

        OUTPUT STORED IN FILE: valbl_log.txt in ./
        */

        std::ofstream log;
        if(DEBURGZERG == 1)
        {
            log.open("./valbl_log.txt");
            if(log.fail())
            {
                std::cout << "Error: opening valbl_log.txt failed" << std::endl;
                return false;
            }
            log << "Writing this to file \n"
        }

        int workers_alive = this->global_id_ - 1;
        int supply = 11 - workers_alive;
        std::vector<std::string> build_successful;

        //flag to indicate period where vespene can be gathered (i.e refinery built and at least one scv available)

        bool vespene_gathered = false;

        //add workers to build_successful, also add basis building
        for(int i = 0; i<workers_alive; ++i)
            { build_successful.push_back("scv"); }

        build_successful.push_back("command_center");

        //print initial build to log
        if(DEBURGZERG == 1)
        {
            log << "initial build \n\n";
            for(std::string obj : build_successful) { log << obj << "\n"; }

            log << "initial supply: \t\t" << supply << "\n";
            log << "initial number workers: \t\t" << workers_alive << "\n\n";
        }

        //iterate over all commands checking if prerequisites are met each time
        for(auto it = list.cbegin(); it != list.cend(); ++it)
        {
            if(DEBURGZERG == 1)
                log << "\t\t >>>BUILDING<<<\n" << "build " << *it << "\n\n";

            //empty command indicates (ifIFIF) last line of file
            if(it->compare("") == 0)
            {
                if(DEBURGZERG == 1)
                    log << "found empty line (assumning buildlist is finished) \n"
                break;
            }

            //lookup command in tech_tree_, if not found, something went wrong
            auto curr_entry_it = tech_tree_.find(*it);
            if(curr_entry_it == tech_tree_.cend())
            {
                if(DEBURGZERG == 1)
                    {
                        log << "Error: Could not find given comman in techtree\n" << "\t\t>>>BUILDLIST INVALID<<<" <<
                        std::endl;
                        log.close();
                    }
                    return false;
            }

            //at this point, the command given is considered valid
            //so, check dependencies_, producers and vespene requirements/availability (also only two refineries per game

            //Testing:
            //check base_buildings and refinery
            if(it->compare("refinery") == 0)
            {
                int count_base_building = 0;
                int count_extractors = 0;

                for(std::string e : build_successful)
                {
                    if(e.compare("refinery") == 0) //count refinery
                        ++count_extractors;

                    else if(e.compare("command_center") == 0 || e.compare("orbital_command") == 0 || e.compare("planetary_fortress") == 0) //count base building
                        ++count_base_builidng;
                }

                if(count_extractors + 1 <= 2*count_base_building) //valid
                {
                    if(DEBURGZERG == 1)
                    {
                        log << "trying to build refinery\n already owning " << count_extractors << "refinery, found" <<
                        count_base_building << "base_buildings\n\n refinery can be built" << std::endl;
                    }
                }
                else //INVALID
                {
                    if(DEBURGZERG == 1)
                    {
                        log << "Error: Refinery can't be built\n tried for " << count_extractors + 1 << "refinery but only found " <<
                        "base buildings!\n \t\t>>>BUILDLIST INVALID<<<" << std::endl;
                        log.close();
                    }
                    return false;
                }
            }

            std::vector<std::string> producers = curr_entry_it->second.getProducers();
            std::vector<std::string> dependencies = curr_entry_it->second.getDependencies();

            size_t pos = 0;

            //Testing
            //check for dependency
            bool dep_met = false;
            for(unsigned int i = 0; i<dependencies.size(); ++i)
            {
                if(dependencies.at(0).compare("") == 0)
                {
                    dep_met = true;
                    break;
                }
                if(std::find(build_successful.cbegin(), build_successful.cend(), dependencies.at(i)) != build_successful.cend())
                {
                    dep_met = true;
                    break;
                }
            }

            if(!dep_met)
            {
                if(DEBURGZERG == 1)
                    {
                        log << "Error: Dependencies not met " << "\ndependencies_: [";
                        for(std::string dep : dependencies) { log << dep << "\t"; }
                        log << "]\n \t\t>>>BUILDLIST INVALID<<<" << std::endl;
                        log.close();
                    }
                    return false;
            }

            //if unit costs vespene but there's no extractor
            if(curr_entry_it->second.getVespeneCost() > 0 && !vespene_gathered )
            {
                if(DEBURGZERG == 1)
                {
                    log << "vespene required\n for building " << curr_entry_it->second.getVespeneCost() <<
                    ", vespene is required but it can't be gathered for now\n";
                    log << "\t\t>>>BUILDLIST INVALID<<<" << std::endl;
                    log.close();
                }
                return false;
            }

            //Testing
            //check for producer
            if(std::find(producers.cbegin(), producers.cend(), "scv") != producers.cend())
            {
                if(workers_alive > 0)
                {
                    build_successful.push_back(*it);
                    supply += curr_entry_it->second.getSupplyProvide();
                    supply -= curr_entry_it->second.getSupplyCost();

                    if(DEBURGZERG == 1)
                    {
                        log << "build requirement (scv) met\n\n \t\t>>>BUILD SUCCESSFUL<<< \n new supply: " << supply <<
                        "\n new workers_alive: "<< workers_alive << "\n\n";
                    }
                }

                else
                {
                    if(DEBURGZERG == 1)
                    {
                        log << "no scv availabe? Something went wrong! \n" << "\t\t>>>BUILDLIST INVALID<<<\n" <<
                        "workers_alive: " << workers_alive << "\n\n";
                    }
                }
            }

            else if(it->compare("orbital_command") == 0 || it->compare("planetary_fortress") == 0)
            {
                //check if builiding to be built is an upgrade for command_center
                if(find(build_successful.cbegin(), build_successful.cend(), producers.at(0)) != build_successful.cend())
                {
                    build_successful.push_back(*it);
                    supply += curr_entry_it->second.getSupplyProvide();
                    supply -= curr_entry_it->second.getSupplyCost();

                    if(DEBURGZERG == 1)
                    {
                        log << "built orbital command or planetary fortress \n \t\t>>>BUILD SUCCESSFUL<<< \n new supply: " << supply <<
                        "\n new workers_alive: "<< workers_alive << "\n\n";
                    }
                }

                else
                {
                    if(DEBURGZERG == 1)
                        log << "building orbital command or planetary fortress failed. no base building availabe\n \t\t>>>BUILDLIST INVALID<<<\n";
                }
            }

            else if((pos = it->std::string::find("_with_tech_lab")) != std::string::npos || (pos = it->std::string::find("_with_reactor")) != std::string::npos)
            {
                //a xyz_with_tech_lab or xyz_with_reactor should be built
                auto morph_it = find(build_successful.cbegin(), build_successful.cend(), producers.at(0));

                if(morph_it == build_successful.cend())
                {
                    if(DEBURGZERG == 1)
                    {
                        log << "Error: Upgrading not possible\n found nothing to upgrade: " << producers.at(0) <<
                        "is required \n \t\t>>>BUILDLIST INVALID<<<" << std::endl;
                        log.close();
                    }
                    return false;
                }

                else //morph unit into new one with techlab or reactor
                {
                    std::string morpher = *morph_it;
                    build_successful.erase(morph_it);
                    build_successful.push_back(*it);
                    supply += curr_entry_it->second.getSupplyProvide();
                    supply -= curr_entry_it->second.getSupplyCost();

                    if(DEBURGZERG == 1)
                    {
                        log << "produced unit by: " *morph_it << "\n \t\t>>>BUILD SUCCESSFUL<<< \n new supply: " << supply <<
                        "\n new workers_alive: "<< workers_alive << "\n\n";
                    }
                }
            }

            else // normal building process for all other units
            {
                auto build_it = build_successful.cend();
                for(std::string prod : producers) //check all possible producers in build list
                {
                    build_it = find(build_successful.cbegin(), build_successful.cend(), prod);

                    if(build_it != build_successful.cend()) //found producer!
                        break;
                }

                if(build_it == build_successful.cend()) //couldn't find producer
                {
                    if(DEBURGZERG == 1)
                    {
                        log << "Building not possible\n found nothing to produce: " << producers.at(0) <<
                        "is required \n \t\t>>>BUILDLIST INVALID<<<" << std::endl;
                        log.close();
                    }
                    return false;
                }

                else
                {
                    //producer found, execute building
                    build_successful.push_back(*it);

                    if(it->compare("scv") == 0) //if scv built, workers available count increases
                        ++workers_alive;

                    supply += curr_entry_it->second.getSupplyProvide();
                    supply -= curr_entry_it->second.getSupplyCost();

                    if(DEBURGZERG == 1)
                    {
                        log << "produced unit by: " *build_it << "\n \t\t>>>BUILD SUCCESSFUL<<< \n new supply: " << supply <<
                        "\n new workers_alive: "<< workers_alive << "\n\n";
                    }
                }
            }

            if(supply < 0)
            {
                if(DEBURGZERG == 1)
                {
                    log << "Supply not sufficient\n Supply went negative while building\n \t\t>>>BUILDLIST INVALID<<<" << std::endl;
                    log.close();
                }
                return false;
            }

            //check if vespene could be gathered after this round
            if(find(build_successful.cbegin(), build_successful.cend(), "refinery") != build_successful.cend() &&
                find(build_successful.cbegin(), build_successful.cend(), "scv") != build_successful.cend())
                { vespene_gathered = true; }
        }

        //End of iteration. If this point is reached, then buildlist is considered valid
        if(DEBURGZERG == 1)
        {
            log << "\n\n \t\t>>>BUILDLIST VALID<<<" << std::endl;
            log.close();
        }
        return true;
    }
};
