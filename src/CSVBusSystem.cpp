#include "../include/CSVBusSystem.h"
#include "../include/BusSystem.h"
#include <iostream>

struct CStop : public CBusSystem::SStop {
public:
    CStop(CBusSystem::TStopID id, CStreetMap::TNodeID nodeId) : id_(id), nodeId_(nodeId) {}

    CBusSystem::TStopID ID() const noexcept override { return id_; }
    CStreetMap::TNodeID NodeID() const noexcept override { return nodeId_; }

private:
    CBusSystem::TStopID id_;
    CStreetMap::TNodeID nodeId_;
};

// Define the concrete CRoute class
struct CRoute : public CBusSystem::SRoute {
public:
    CRoute(const std::string& name, CBusSystem::TStopID stopId) : name_(name), stopIds_({stopId}) {} // Initialize with a single stop ID

    std::string Name() const noexcept override { return name_; }
    std::size_t StopCount() const noexcept override { return stopIds_.size(); }
    CBusSystem::TStopID GetStopID(std::size_t index) const noexcept override {
        if (index < stopIds_.size()) {
            return stopIds_[index];
        } else {
            return CBusSystem::InvalidStopID;
        }
    }

private:
    std::string name_;
    std::vector<CBusSystem::TStopID> stopIds_; // Store stop IDs in a vector
};

struct CCSVBusSystem::SImplementation {
    // ... SImplementation members ...
    std::shared_ptr<CDSVReader> stopReader;
    std::shared_ptr<CDSVReader> routeReader;
    std::vector<std::shared_ptr<CStop>> stops;
    std::vector<std::shared_ptr<CRoute>> routes;
    SImplementation() {} 
};


CCSVBusSystem::CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc)
    : DImplementation(std::make_unique<SImplementation>()) { // Initialize DImplementation

    DImplementation->stopReader = stopsrc;
    DImplementation->routeReader = routesrc;
    //std::vector<std::shared_ptr<CStop>> stops;
    //std::vector<std::shared_ptr<CRoute>> routes;

    //Reading from first row of both routes and stops so I skip the first row 
    try {
        std::vector<std::string> rowStop;
        stopsrc->ReadRow(rowStop);
        std::vector<std::string> rowRoute;
        routesrc->ReadRow(rowRoute);
    } catch (const std::invalid_argument& e) {
        std::cerr << "could not read routes: " << e.what() << std::endl;
    }
    //Now reading from the rest and populating my stops and routes
    while (!stopsrc->End()) {
        try {
            std::vector<std::string> row;
            stopsrc->ReadRow(row);
            TStopID stopId = std::stoul(row[0]);
            CStreetMap::TNodeID nodeId = std::stoul(row[1]);
            auto stop = std::make_shared<CStop>(stopId, nodeId);
            DImplementation->stops.push_back(stop);
        } catch (const std::invalid_argument& e) {
            std::cerr << "could not read stops: " << e.what() << std::endl;
        }
    }
    while (!routesrc->End()) {
        try {
            std::vector<std::string> row;
            stopsrc->ReadRow(row);
            std::string name = row[0];
            TStopID stopId = std::stoul(row[1]);
            auto route = std::make_shared<CRoute>(name, stopId);
            DImplementation->routes.push_back(route);
        } catch (const std::invalid_argument& e) {
            std::cerr << "could not read routes: " << e.what() << std::endl;
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
    for (std::size_t i = 0; i < DImplementation->stops.size(); i++) {
        if (DImplementation->stops[i]->ID() == id) {
            return DImplementation->stops[i];
        }
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

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string& name) const noexcept {
    for (std::size_t i = 0; i < DImplementation->routes.size(); i++) {
        if (DImplementation->routes[i]->Name() == name) {
            return DImplementation->routes[i];
        }
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