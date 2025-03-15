#include "../include/BusSystemIndexer.h"
#include "../include/BusSystem.h"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm> // Use this instead of bits/stdc++.h

struct CBusSystemIndexer::SImplementation {
    std::shared_ptr<CBusSystem> bus_system;
    
    SImplementation(std::shared_ptr<CBusSystem> bussystem) {
        bus_system = bussystem;
    }
};

CBusSystemIndexer::CBusSystemIndexer(std::shared_ptr<CBusSystem> bussystem) {
    DImplementation = std::make_unique<SImplementation>(bussystem);
}

CBusSystemIndexer::~CBusSystemIndexer() {}

std::size_t CBusSystemIndexer::StopCount() const noexcept {
    auto stop_count = DImplementation->bus_system->StopCount();
    return stop_count;
}

std::size_t CBusSystemIndexer::RouteCount() const noexcept {
    auto route_count = DImplementation->bus_system->RouteCount();
    return route_count;
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SortedStopByIndex(std::size_t index) const noexcept {
    if (DImplementation->bus_system->StopCount() < index){
        return nullptr;
    }
    
    std::vector<CBusSystem::TStopID> stop_ids;
    for (std::size_t i = 0; i < DImplementation->bus_system->StopCount(); i++) {
        auto stop = DImplementation->bus_system->StopByIndex(i);
        if (stop) {
            stop_ids.push_back(stop->ID());
        }
    }
    
    std::sort(stop_ids.begin(), stop_ids.end());
    if (index < stop_ids.size()) {
        return DImplementation->bus_system->StopByID(stop_ids[index]);
    }
    return nullptr;
}

std::shared_ptr<CBusSystemIndexer::SRoute> CBusSystemIndexer::SortedRouteByIndex(std::size_t index) const noexcept {
    if (index >= DImplementation->bus_system->RouteCount()) return nullptr;
    
    std::vector<std::string> route_ids;
    for (std::size_t i = 0; i < DImplementation->bus_system->RouteCount(); i++) {
        auto route = DImplementation->bus_system->RouteByIndex(i);
        if (route) {
            route_ids.push_back(route->Name());
        }
    }
    
    std::sort(route_ids.begin(), route_ids.end());
    if (index < route_ids.size()) {
        return DImplementation->bus_system->RouteByName(route_ids[index]);
    }
    return nullptr;
}

std::shared_ptr<CBusSystemIndexer::SStop> CBusSystemIndexer::StopByNodeID(TNodeID id) const noexcept {
    for (std::size_t i = 0; i < DImplementation->bus_system->StopCount(); i++) {
        auto stop = DImplementation->bus_system->StopByIndex(i);
        if (stop && stop->NodeID() == id) {
            return stop;
        }
    }
    return nullptr;
}

bool CBusSystemIndexer::RoutesByNodeIDs(CBusSystemIndexer::TNodeID src, CBusSystemIndexer::TNodeID dest, std::unordered_set<std::shared_ptr<CBusSystemIndexer::SRoute>> &routes) const noexcept {
    auto src_stop = StopByNodeID(src);
    auto dest_stop = StopByNodeID(dest);
    
    if (!src_stop || !dest_stop) {
        return false;
    }
    
    CBusSystem::TStopID src_stop_id = src_stop->ID();
    CBusSystem::TStopID dest_stop_id = dest_stop->ID();
    
    for (std::size_t i = 0; i < DImplementation->bus_system->RouteCount(); i++) {
        auto route = DImplementation->bus_system->RouteByIndex(i);
        if (!route) continue;
        
        bool found_src = false;
        bool found_dest = false;
        
        for (std::size_t j = 0; j < route->StopCount(); j++) {
            if (route->GetStopID(j) == src_stop_id) {
                found_src = true;
            }
            if (route->GetStopID(j) == dest_stop_id) {
                found_dest = true;
            }
            
            // If we found both stops, add the route to the set
            if (found_src && found_dest) {
                routes.insert(route);
                break;
            }
        }
    }
    return !routes.empty();
}

bool CBusSystemIndexer::RouteBetweenNodeIDs(CBusSystemIndexer::TNodeID src, CBusSystemIndexer::TNodeID dest) const noexcept {
    std::unordered_set<std::shared_ptr<CBusSystemIndexer::SRoute>> routes;
    return RoutesByNodeIDs(src, dest, routes);
}
/*
#include "BusSystemIndexer.h"
#include "BusSystem.h"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm> // Use this instead of bits/stdc++.h

struct CBusSystemIndexer::SImplementation {
    std::shared_ptr<CBusSystem> BusSystem;
    
    SImplementation(std::shared_ptr<CBusSystem> bussystem) {
        BusSystem = bussystem;
    }
    
    std::size_t StopCount() const {
        return BusSystem->StopCount();
    }
    
    std::size_t RouteCount() const {
        return BusSystem->RouteCount();
    }
    
    std::shared_ptr<SStop> SortedStopByIndex(std::size_t index) const {
        if (index >= BusSystem->StopCount()) return nullptr;
        
        std::vector<CBusSystem::TStopID> StopID;
        for (std::size_t i = 0; i < BusSystem->StopCount(); i++) {
            auto stop = BusSystem->StopByIndex(i);
            if (stop) {
                StopID.push_back(stop->ID());
            }
        }
        
        std::sort(StopID.begin(), StopID.end());
        if (index < StopID.size()) {
            return BusSystem->StopByID(StopID[index]);
        }
        return nullptr;
    }
    
    std::shared_ptr<SRoute> SortedRouteByIndex(std::size_t index) const {
        if (index >= BusSystem->RouteCount()) return nullptr;
        
        std::vector<std::string> RouteID;
        for (std::size_t i = 0; i < BusSystem->RouteCount(); i++) {
            auto route = BusSystem->RouteByIndex(i);
            if (route) {
                RouteID.push_back(route->Name());
            }
        }
        
        std::sort(RouteID.begin(), RouteID.end());
        if (index < RouteID.size()) {
            return BusSystem->RouteByName(RouteID[index]);
        }
        return nullptr;
    }
    
    std::shared_ptr<SStop> StopByNodeID(TNodeID id) const {
        for (std::size_t i = 0; i < BusSystem->StopCount(); i++) {
            auto stop = BusSystem->StopByIndex(i);
            if (stop && stop->NodeID() == id) {
                return stop;
            }
        }
        return nullptr;
    }
    
    bool RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<SRoute>> &routes) const {
        auto srcStop = StopByNodeID(src);
        auto destStop = StopByNodeID(dest);
        
        if (!srcStop || !destStop) {
            return false;
        }
        
        CBusSystem::TStopID srcStopID = srcStop->ID();
        CBusSystem::TStopID destStopID = destStop->ID();
        
        for (std::size_t i = 0; i < BusSystem->RouteCount(); i++) {
            auto route = BusSystem->RouteByIndex(i);
            if (!route) continue;
            
            bool foundSrc = false;
            bool foundDest = false;
            
            for (std::size_t j = 0; j < route->StopCount(); j++) {
                if (route->GetStopID(j) == srcStopID) {
                    foundSrc = true;
                }
                if (route->GetStopID(j) == destStopID) {
                    foundDest = true;
                }
                
                // If we found both stops, add the route to the set
                if (foundSrc && foundDest) {
                    routes.insert(route);
                    break;
                }
            }
        }
        
        return !routes.empty();
    }
    
    bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const {
        std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> Routes;
        return RoutesByNodeIDs(src, dest, Routes);
    }
};
/*
CBusSystemIndexer::CBusSystemIndexer(std::shared_ptr<CBusSystem> bussystem) {
    DImplementation = std::make_unique<SImplementation>(bussystem);
}

CBusSystemIndexer::~CBusSystemIndexer() {}

std::size_t CBusSystemIndexer::StopCount() const noexcept {
    return DImplementation->StopCount();
}

std::size_t CBusSystemIndexer::RouteCount() const noexcept {
    return DImplementation->RouteCount();
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SortedStopByIndex(std::size_t index) const noexcept {
    return DImplementation->SortedStopByIndex(index);
}

std::shared_ptr<CBusSystem::SRoute> CBusSystemIndexer::SortedRouteByIndex(std::size_t index) const noexcept {
    return DImplementation->SortedRouteByIndex(index);
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::StopByNodeID(TNodeID id) const noexcept {
    return DImplementation->StopByNodeID(id);
}

bool CBusSystemIndexer::RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> &routes) const noexcept {
    return DImplementation->RoutesByNodeIDs(src, dest, routes);
}

bool CBusSystemIndexer::RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept {
    return DImplementation->RouteBetweenNodeIDs(src, dest);
}
*/