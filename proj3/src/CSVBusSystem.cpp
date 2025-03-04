#include "CSVBusSystem.h"  //this includes the class definition for CCSVBusSystem
#include "DSVReader.h"    //reading CSV (or other DSV) formatted input files
#include <memory>           //provides std::shared_ptr and std::make_shared for memory management
#include <vector>           //used to store lists of stops and routes
#include <string>           //enables usage of std::string for the route name and attribute
#include <unordered_map>    //lookup of stops and routes by ID or name using unordered_map
#include <iostream>         //i need to print bus system details using operator <<

//defines the SStop class, representing a bus stop, inheriting from CBusSystem::SStop can be seen in header file
class CCSVBusSystem::SStop : public CBusSystem::SStop {
public:
    TStopID StopID;  // unique identifier for the stop
    CStreetMap::TNodeID NodeIDValue;  // corresponding node ID in the street map

    //returns the stop ID
    TStopID ID() const noexcept override {
        return StopID;
    }

    //returns the node ID associated with this stop
    CStreetMap::TNodeID NodeID() const noexcept override {
        return NodeIDValue;
    }
};

// defines the SRoute class, representing a bus route, inheriting from CBusSystem::SRoute
class CCSVBusSystem::SRoute : public CBusSystem::SRoute {
public:
//name of the route
    std::string RouteName;  
//list of stop IDs forming the route
    std::vector<TStopID> RouteStops;  

    //return the route name
    std::string Name() const noexcept override {
        return RouteName;
    }

    // return the number of stops in the route
    std::size_t StopCount() const noexcept override {
        return RouteStops.size();
    }

    // return the stop ID at the given index
    TStopID GetStopID(std::size_t index) const noexcept override {
        if (index >= RouteStops.size()) {
            // return an invalid ID if index is out of bounds
            return CBusSystem::InvalidStopID;  
        }
    }

private:
    std::string name_;
    std::vector<CBusSystem::TStopID> stopIds_; // Store stop IDs in a vector
};

// defines the internl impl structure
struct CCSVBusSystem::SImplementation {
    // ... SImplementation members ...
    std::shared_ptr<CDSVReader> stopReader;
    std::shared_ptr<CDSVReader> routeReader;
    std::vector<std::shared_ptr<SStop>> StopsByIndex;  
    std::unordered_map<TStopID, std::shared_ptr<SStop>> Stops; 
    std::vector<std::shared_ptr<SRoute>> RoutesByIndex; 
    std::unordered_map<std::string, std::shared_ptr<SRoute>> Routes;  
};

// constructor for the bus system
CCSVBusSystem::CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc) 
    : DImplementation(std::make_unique<SImplementation>()) {
    
    DImplementation->stopReader = stopsrc;
    DImplementation->routeReader = routesrc;
    std::vector<std::string> row;  
    
    //read stops data
    if (stopsrc) {
        //read each row of the stop file with a while loop
        while (stopsrc->ReadRow(row)) {
            // ensure sufficient columns exists
            if (row.size() >= 2) {  
                try {
                    auto stop = std::make_shared<SStop>();
                    //convert first column to StopID(0 is first in index)
                    stop->StopID = std::stoul(row[0]);  
                    //convert second column to NodeID(1 is second in index)
                    stop->NodeIDValue = std::stoul(row[1]);  
                    //store in map for quick lookup
                    DImplementation->Stops[stop->StopID] = stop; 
                    //store in list for indexed access 
                    DImplementation->StopsByIndex.push_back(stop);  
                } catch (const std::exception& e) {
                    //handle error by 
                    std::cerr << "Exception caught: " << e.what() << "\n";
                }
            }
        }
    }

    // read routes data
    if (routesrc) {
        // tmp storage for routes
        std::unordered_map<std::string, std::shared_ptr<SRoute>> tempRoutes;  
       // read each row of the route file
        while (routesrc->ReadRow(row)) {  
            // ensure sufficient columns exist
            if (row.size() >= 2) {  
                try {
                    //first column is route name
                    std::string routeName = row[0];  
                    //second column is StopID
                    TStopID stopID = std::stoul(row[1]);  
                 // retrieve or create route entry
                    auto& route = tempRoutes[routeName];  
                    if (!route) {
                        route = std::make_shared<SRoute>();
                        route->RouteName = routeName;
                    }
                    // Append stop to route by push_back
                    route->RouteStops.push_back(stopID);  
                } catch (const std::exception& e) {
                    //handle error by 
                    std::cerr << "Exception caught: " << e.what() << "\n";
                }
            }
        }

        for (const auto& pair : tempRoutes) {
            // store in route map
            DImplementation->Routes[pair.first] = pair.second; 
            // store in index list 
            DImplementation->RoutesByIndex.push_back(pair.second);  
        }
    }
}

// destructor
CCSVBusSystem::~CCSVBusSystem() = default;

// return the total number of stops
std::size_t CCSVBusSystem::StopCount() const noexcept {
    return DImplementation->StopsByIndex.size();
}

// return the total number of routes
std::size_t CCSVBusSystem::RouteCount() const noexcept {
    return DImplementation->RoutesByIndex.size();
}

// return a stop by index
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept {
    if (index < DImplementation->StopsByIndex.size()) {
        return DImplementation->StopsByIndex[index];
    }
    return nullptr;
}

// return a stop by its ID
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(TStopID id) const noexcept {
    auto it = DImplementation->Stops.find(id);
    if (it != DImplementation->Stops.end()) {
        return it->second;
    }
    return nullptr;
}

// return a route by index
std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept {
    if (index < DImplementation->RoutesByIndex.size()) {
        return DImplementation->RoutesByIndex[index];
    }
    return nullptr;
}

// return a route by name
std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept {
    auto it = DImplementation->Routes.find(name);
    if (it != DImplementation->Routes.end()) {
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
