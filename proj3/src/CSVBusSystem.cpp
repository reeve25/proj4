#include "CSVBusSystem.h"
#include "DSVReader.h"
#include <vector>
#include <memory>
#include <iostream>   
#include <string>
#include <unordered_map>

class CCSVBusSystem::SStop : public CBusSystem::SStop {
public:

    CStreetMap::TNodeID nodeId_; 
    CBusSystem::TStopID id_;

    CStreetMap::TNodeID NodeID() const noexcept override { return nodeId_; }

    TStopID ID() const noexcept override { return id_; }
};

struct CCSVBusSystem::SRoute : public CBusSystem::SRoute {
public:
    std::string name_;
    std::vector<TStopID> stopIds_;  

    std::string Name() const noexcept override { return name_; }

    std::size_t StopCount() const noexcept override { return stopIds_.size(); }

    CBusSystem::TStopID GetStopID(std::size_t index) const noexcept override {
        if (index < stopIds_.size()) {
            return stopIds_[index];
        } else {
            return CBusSystem::InvalidStopID;
        }
    }
};

struct CCSVBusSystem::SImplementation {
    // ... SImplementation members ...
    std::vector<std::shared_ptr<SStop>> stops;
    std::unordered_map<TStopID, std::shared_ptr<SStop>> stopmap;
    std::vector<std::shared_ptr<SRoute>> routes;
    std::unordered_map<std::string, std::shared_ptr<SRoute>> routemap;  
    //SImplementation() {} 
};

CCSVBusSystem::CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc)    
    : DImplementation(std::make_unique<SImplementation>()) {
    std::vector<std::string> row;  


    if (stopsrc) {
        while (stopsrc->ReadRow(row)) {
            try {
                auto stop = std::make_shared<SStop>();
                stop->id_ = std::stoul(row[0]);  
                stop->nodeId_ = std::stoul(row[1]);
                DImplementation->stopmap.emplace(stop->id_, stop);
                DImplementation->stops.emplace_back(stop);  
            } catch (const std::exception& e) {
                std::cerr << "exception caught: " << e.what() << "\n";
            }
        }
    }

    if (routesrc) {
        std::unordered_map<std::string, std::shared_ptr<SRoute>> temporaryRoutes;  
        while (routesrc->ReadRow(row)) {  
            if (row.size() >= 2) {  
                try {
                    std::string routeName = row[0];  
                    TStopID stopID = std::stoul(row[1]);  
                    auto& route = temporaryRoutes[routeName];  
                    if (!route) {
                        route = std::make_shared<SRoute>();
                        route->name_ = routeName;
                    }
                    route->stopIds_.push_back(stopID);  
                } catch (const std::exception& e) {
                    //handle error
                    std::cerr << "exception caught: " << e.what() << "\n";
                }
            }
        }

        for (auto it = temporaryRoutes.begin(); it != temporaryRoutes.end(); ++it) {
            DImplementation->routemap[it->first] = it->second;
            DImplementation->routes.push_back(it->second);  
        }
    }
}

// Destructor
CCSVBusSystem::~CCSVBusSystem() = default;

std::size_t CCSVBusSystem::StopCount() const noexcept {
    return DImplementation->stops.size();
}

std::size_t CCSVBusSystem::RouteCount() const noexcept {
    return DImplementation->routes.size();
}

// return stops by index
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept {
    if (index >= DImplementation->stops.size()) {
        return nullptr;
    } else {
        return DImplementation->stops[index];
    }
}

// return stopmap by id
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(TStopID id) const noexcept {
    auto it = DImplementation->stopmap.find(id);
    if (it != DImplementation->stopmap.end()) {
        return it->second;
    }
    return nullptr;
}

// return routes by their index
std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept {
    if (index >= DImplementation->routes.size()) {
        return nullptr;
    } else {
        return DImplementation->routes[index];
    }
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept {
    auto it = DImplementation->routemap.find(name);
    if (it != DImplementation->routemap.end()) {
        return it->second;
    }
    return nullptr;
}

// CCSVBusSystem member functions
// Constructor for the CSV Bus System
    //CCSVBusSystem(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc);
// Destructor for the CSV Bus System
    //~CCSVBusSystem();
// Returns the number of stops in the system
    //std::size_t StopCount() const noexcept override;
// Returns the number of routes in the system
    //std::size_t RouteCount() const noexcept override;
// Returns the SStop specified by the index, nullptr is returned if index is
// greater than equal to StopCount()
    //std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept override;
// Returns the SStop specified by the stop id, nullptr is returned if id is
// not in the stops
    //std::shared_ptr<SStop> StopByID(TStopID id) const noexcept override;
// Returns the SRoute specified by the index, nullptr is returned if index is
// greater than equal to RouteCount()
    //std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept
    //override;
// Returns the SRoute specified by the name, nullptr is returned if name is
// not in the routes
    //std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept
    //override;
// Bus System Stop member functions
// Returns the stop id of the stop
    //TStopID ID() const noexcept override;
// Returns the node id of the bus stop
    //CStreetMap::TNodeID NodeID() const noexcept override;
// Bus System Route member functions
// Returns the name of the route
    //std::string Name() const noexcept override;
// Returns the number of stops on the route
    //std::size_t StopCount() const noexcept override;
// Returns the stop id specified by the index, returns InvalidStopID if index
// is greater than or equal to StopCount() 
//TStopID GetStopID(std::size_t index) const noexcept override;