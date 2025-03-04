#include "CSVBusSystem.h"
#include "DSVReader.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

class CCSVBusSystem::SStop : public CBusSystem::SStop {
public:
    TStopID StopID;
    CStreetMap::TNodeID NodeIDValue;

    TStopID ID() const noexcept override {
        return StopID;
    }

    CStreetMap::TNodeID NodeID() const noexcept override {
        return NodeIDValue;
    }
};

class CCSVBusSystem::SRoute : public CBusSystem::SRoute {
public:
    std::string RouteName;
    std::vector<TStopID> RouteStops;  

    std::string Name() const noexcept override {
        return RouteName;
    }

    std::size_t StopCount() const noexcept override {
        return RouteStops.size();
    }

    TStopID GetStopID(std::size_t index) const noexcept override {
        if (index >= RouteStops.size()) {
            return CBusSystem::InvalidStopID;  
        }
        return RouteStops[index];
    }
};

struct CCSVBusSystem::SImplementation {
    std::shared_ptr<CDSVReader> stopReader;
    std::shared_ptr<CDSVReader> routeReader;
    std::vector<std::shared_ptr<SStop>> stops;  
    std::unordered_map<TStopID, std::shared_ptr<SStop>> stopMap; 
    std::vector<std::shared_ptr<SRoute>> routes; 
    std::unordered_map<std::string, std::shared_ptr<SRoute>> routeMap;  
};

CCSVBusSystem::CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc) 
    : DImplementation(std::make_unique<SImplementation>()) {
    
    DImplementation->stopReader = stopsrc;
    DImplementation->routeReader = routesrc;
    std::vector<std::string> row;  
    
    if (stopsrc) {
        while (stopsrc->ReadRow(row)) {
            if (row.size() >= 2) {  
                try {
                    auto stop = std::make_shared<SStop>();
                    stop->StopID = std::stoul(row[0]);
                    stop->NodeIDValue = std::stoul(row[1]);
                    DImplementation->stopMap[stop->StopID] = stop; 
                    DImplementation->stops.push_back(stop);  
                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << "\n";
                }
            }
        }
    }

    if (routesrc) {
        std::unordered_map<std::string, std::shared_ptr<SRoute>> tempRoutes;  
        while (routesrc->ReadRow(row)) {
            if (row.size() >= 2) {  
                try {
                    std::string routeName = row[0];
                    TStopID stopID = std::stoul(row[1]);
                    auto& route = tempRoutes[routeName];  
                    if (!route) {
                        route = std::make_shared<SRoute>();
                        route->RouteName = routeName;
                    }
                    route->RouteStops.push_back(stopID);  
                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << "\n";
                }
            }
        }

        for (const auto& pair : tempRoutes) {
            DImplementation->routeMap[pair.first] = pair.second;
            DImplementation->routes.push_back(pair.second);  
        }
    }
}
CCSVBusSystem::~CCSVBusSystem() = default;

std::size_t CCSVBusSystem::StopCount() const noexcept {
    return DImplementation->stops.size();
}

std::size_t CCSVBusSystem::RouteCount() const noexcept {
    return DImplementation->routes.size();
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept {
    if (index >= DImplementation->stops.size()) {
        return nullptr;
    } else {
        return DImplementation->stops[index];
    }
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(TStopID id) const noexcept {
    auto currStop = DImplementation->stopMap.find(id);
    if (currStop != DImplementation->stopMap.end()) {
        return currStop->second;
    }
    return nullptr;
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept {
    if (index >= DImplementation->routes.size()) {
        return nullptr;
    } else {
        return DImplementation->routes[index];
    }
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept {
    auto it = DImplementation->routeMap.find(name);
    if (it != DImplementation->routeMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::ostream &operator<<(std::ostream &os, const CCSVBusSystem &bussystem) {
    os << "amt of stops: " << std::to_string(bussystem.StopCount()) << "\n";
    os << "amt of routes: " << std::to_string(bussystem.RouteCount()) << "\n";
    
    for (size_t i = 0; i < bussystem.StopCount(); i++) {
        auto stop = bussystem.StopByIndex(i);
        if (stop) {
            os << "index " << std::to_string(i) << " ID: " << std::to_string(stop->ID()) <<
                  " NodeID: " << std::to_string(stop->NodeID()) << "\n";
        }
    }
    
    for (size_t i = 0; i < bussystem.RouteCount(); i++) {
        auto route = bussystem.RouteByIndex(i);
        if (route) {
            os << "index route " << std::to_string(i) << " name: " << route->Name() +
                  " amount of stops: " << std::to_string(route->StopCount()) << "\n";
        }
    }
    
    return os;
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
