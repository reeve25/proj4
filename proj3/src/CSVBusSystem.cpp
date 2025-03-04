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
        while (!stopsrc->End()) {
            try {
                auto stop = std::make_shared<SStop>();
                stop->id_ = std::stoul(row[0]);  
                stop->nodeId_ = std::stoul(row[1]);
                DImplementation->stopmap[stop->id_] = stop; 
                DImplementation->stops.push_back(stop);  
            } catch (const std::exception& e) {
                std::cerr << "Exception caught: " << e.what() << "\n";
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
                        route->name_ = routeName;
                    }
                    route->stopIds_.push_back(stopID);  
                } catch (const std::exception& e) {
                    //handle error
                    std::cerr << "exception caught: " << e.what() << "\n";
                }
            }
        }

        for (const auto& pair : tempRoutes) {
            DImplementation->routemap[pair.first] = pair.second;
            DImplementation->routes.push_back(pair.second);  
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
    if (index < DImplementation->stops.size()) {
        return DImplementation->stops[index];
    }
    return nullptr;
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
    if (index < DImplementation->routes.size()) {
        return DImplementation->routes[index];
    }
    return nullptr;
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept {
    auto it = DImplementation->routemap.find(name);
    if (it != DImplementation->routemap.end()) {
        return it->second;
    }
    return nullptr;
}

std::ostream &operator<<(std::ostream &os, const CCSVBusSystem &bussystem) {
    os << "StopCount: " << std::to_string(bussystem.StopCount()) << "\n";
    os << "RouteCount: " << std::to_string(bussystem.RouteCount()) << "\n";
    
    for (size_t i = 0; i < bussystem.StopCount(); i++) {
        auto stop = bussystem.StopByIndex(i);
        if (stop) {
            os << "Index " << std::to_string(i) << " ID: " << std::to_string(stop->ID()) <<
                  " NodeID: " << std::to_string(stop->NodeID()) << "\n";
        }
    }
    
    for (size_t i = 0; i < bussystem.RouteCount(); i++) {
        auto route = bussystem.RouteByIndex(i);
        if (route) {
            os << "Route Index " << std::to_string(i) << " Name: " << route->Name() +
                  " StopCount: " << std::to_string(route->StopCount()) << "\n";
        }
    }
    
    return os;
}
