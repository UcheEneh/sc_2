#pragma once

#include <iostream>
#include <string>

#include <vector>
#include <cstdlib>

class TechTreeEntry
{
public:
    //splits dependencies and producers into vector
    static std::vector<std::string> split(const std::string str, const char c = "/")
    {
        std::vector<std::string> result;
        size_t str_start = 0;
        size_t pos_delim;

        do
        {
            pos_delim = str.find_first_of(c, str_start);
            if (pos_delim == std::string::npos) { result.push_back(str.substr(str_start, pos_delim)); }

            else { result.push_back(str.substr(str_start, pos_delim-str_start)); }
            str_start = pos_delim + 1;

        } while (pos_delim != std::string::npos);

        return result;
    }

    TechTreeEntry() {}

    //this gets a whole line of the cvs file, already splitting into the different tokens
    TechTreeEntry(const std::vector<std::string>& line):
        name_(line[0]),
        mineral_cost_(std::atoi(line[1].c_str())),
        vespene_cost_(std::atoi(line[2].c_str())),
        build_time_(std::atoi(line[3].c_str())),
        supply_cost_(std::atoi(line[4].c_str())),
        supply_provide_(std::atoi(line[5].c_str())),
        intial_energy_(std::atoi(line[6].c_str())),
        max_energy_(std::atoi(line[7].c_str())),
        morph_((bool)std::atoi(line[11].c_str()))
        {
            producers_ = split(line.at(9));
            dependencies_ = split(line.at(10));
        }

        std::string getName() const { return name_; }
        int getMineralCost() const { return mineral_cost_; }
        int getVespeneCost() const { return vespene_cost_; }
        int getBuildTime() const { return build_time_; }
        int getSupplyCost() const { return supply_cost_; }
        int getSupplyProvide() const { return supply_provide_; }
        int getInitialEnergy() const { return initial_energy_; }
        int getMaxEnergy() const { return max_energy_; }

        const std::vector<std::string>& getProducers() const { return producers_; }
        const std::vector<std::string>& getDependencies() const { return dependencies_; }
        bool getMorph() const { return morph; }

        friend std::ostream& operator<<(std::ostream& str, const TechTreeEntry tte)
        {
            std::cout << tte.name_ << " " << tte.mineral_cost_ << " " << tte.vespene_cost_ << " " << tte.build_time_ << " "
                      << tte.supply_cost_ << " " << tte.supply_provide_ << " " << tte.initial_energy_ << " "
                      << tte.max_energy_ << " " << "producers_: ";

            for (auto it = tte.producers_.cbegin(); it != tte.producers_.cend(); ++it)
            { std::cout << *it << " "; }

            std::cout << "dependencies_: ";
            for (auto it = tte.dependencies_.cbegin(); it != tte.dependencies_.cend(); ++it)
            { std::cout << *it << " "; }

            std::cout << "morphing? " << (tte.morph_ ? "1" : "0") << std::endl;
            return str;
        }

private:
    const std::string name_ = "";
    const int mineral_cost_ = -1;
    const int vespene_cost_ = -1;
    const int build_time_ = -1;
    const int supply_cost_ = -1;
    const int supply_provide_ = -1;
    const int initial_energy_ = -1;
    const int max_energy_ = -1;
    const bool morph_ = false;

    std::vector<std::string> producers_;
    std::vector<std::string> dependencies_;
};
