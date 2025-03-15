#include "BusSystemIndexer.h"       // Header for CBusSystemIndexer class usage
#include "BusSystem.h"              // Header for CBusSystem class usage
#include <memory>                   // For std::shared_ptr and std::make_shared
#include <string>                   // For std::string
#include <iostream>                 // For printing bus system details
#include <vector>                   // To store lists of stops and routes
#include <algorithm>                // To sort stops and routes
#include <unordered_set>            // For storing sets of routes

struct CBusSystemIndexer::SImplementation {
    // Internal pointer to the bus system.
    std::shared_ptr<CBusSystem> busSystemPtr;

    // Constructor.
    SImplementation(std::shared_ptr<CBusSystem> bs);

    // Return the total number of stops.
    std::size_t StopCount() const;

    // Return the total number of routes.
    std::size_t RouteCount() const;

    // Return a stop by its sorted index.
    std::shared_ptr<CBusSystem::SStop> SortedStopByIndex(std::size_t idx) const;

    // Return a route by its sorted index.
    std::shared_ptr<CBusSystem::SRoute> SortedRouteByIndex(std::size_t idx) const;

    // Return a stop by its node ID.
    std::shared_ptr<CBusSystem::SStop> StopByNodeID(TNodeID nodeId) const;

    // Populate a set with routes that pass through both node IDs.
    bool RoutesByNodeIDs(TNodeID src, TNodeID dest,
                         std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> &outRoutes) const;

    // Return true if there is at least one route connecting the two node IDs.
    bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const;
};

// Constructor for the bus system indexer.
CBusSystemIndexer::CBusSystemIndexer(std::shared_ptr<CBusSystem> bs) {
    DImplementation = std::make_unique<SImplementation>(bs);
}

// Destructor for the bus system indexer.
CBusSystemIndexer::~CBusSystemIndexer() {}

// Return the total number of stops.
std::size_t CBusSystemIndexer::StopCount() const noexcept {
    return DImplementation->busSystemPtr->StopCount();
}

// Return the total number of routes.
std::size_t CBusSystemIndexer::RouteCount() const noexcept {
    return DImplementation->busSystemPtr->RouteCount();
}

// Return a stop by sorted index.
std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SortedStopByIndex(std::size_t index) const noexcept {
    if (index >= DImplementation->busSystemPtr->StopCount())
        return nullptr;
    std::vector<CBusSystem::TStopID> sortedStopIDs;
    for (std::size_t i = 0; i < DImplementation->busSystemPtr->StopCount(); ++i) {
        auto stop = DImplementation->busSystemPtr->StopByIndex(i);
        if (stop)
            sortedStopIDs.push_back(stop->ID());
    }
    std::sort(sortedStopIDs.begin(), sortedStopIDs.end());
    if (index < sortedStopIDs.size()) {
        return DImplementation->busSystemPtr->StopByID(sortedStopIDs[index]);
    }
    return nullptr;
}

// Return a route by sorted index.
std::shared_ptr<CBusSystem::SRoute> CBusSystemIndexer::SortedRouteByIndex(std::size_t index) const noexcept {
    if (DImplementation->busSystemPtr->RouteCount() < index) {
        return nullptr;
    }
    std::vector<std::string> routeNames;
    for (std::size_t i = 0; i < DImplementation->busSystemPtr->RouteCount(); ++i) {
        auto route = DImplementation->busSystemPtr->RouteByIndex(i);
        if (route)
            routeNames.push_back(route->Name());
    }
    std::sort(routeNames.begin(), routeNames.end());
    if (index < routeNames.size())
        return DImplementation->busSystemPtr->RouteByName(routeNames[index]);
    return nullptr;
}

// Return a stop by its node ID.
std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::StopByNodeID(TNodeID id) const noexcept {
    std::shared_ptr<CBusSystem::SStop> output = nullptr;
    for (std::size_t i = 0; i < DImplementation->busSystemPtr->StopCount(); ++i) {
        auto stop = DImplementation->busSystemPtr->StopByIndex(i);
        if (stop && stop->NodeID() == id) {
            output = stop;
        }
    }
    return output;
}

// Return the routes between two node IDs.
bool CBusSystemIndexer::RoutesByNodeIDs(TNodeID src, TNodeID dest,
    std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> &routes) const noexcept {
    auto sourceStop = StopByNodeID(src);
    auto destStop = StopByNodeID(dest);
    if (!sourceStop || !destStop) {
        return false;
    }

    CBusSystem::TStopID srcID = sourceStop->ID();
    CBusSystem::TStopID destID = destStop->ID();

    for (std::size_t i = 0; i < DImplementation->busSystemPtr->RouteCount(); ++i) {
        auto route = DImplementation->busSystemPtr->RouteByIndex(i);
        if (route) {
            bool hasSrc = false, hasDest = false;
            for (std::size_t j = 0; j < route->StopCount(); ++j) {
                if (route->GetStopID(j) == srcID)
                    hasSrc = true;
                if (route->GetStopID(j) == destID)
                    hasDest = true;
                if (hasSrc && hasDest) {
                    routes.insert(route);
                    break;
                }
            }
        }
    }
    return routes.size() > 0;
}

// Return true if there is a route between two node IDs.
bool CBusSystemIndexer::RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept {
    std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> tempRoutes;
    return RoutesByNodeIDs(src, dest, tempRoutes);
}

// Definitions of SImplementation functions
CBusSystemIndexer::SImplementation::SImplementation(std::shared_ptr<CBusSystem> bs)
    : busSystemPtr(bs) {}

/*std::size_t CBusSystemIndexer::SImplementation::StopCount() const {
    return busSystemPtr->StopCount();
}

/*std::size_t CBusSystemIndexer::SImplementation::RouteCount() const {
    return busSystemPtr->RouteCount();
}

/*std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SImplementation::SortedStopByIndex(std::size_t idx) const {
    if (idx >= busSystemPtr->StopCount())
        return nullptr;
    std::vector<CBusSystem::TStopID> sortedStopIDs;
    for (std::size_t i = 0; i < busSystemPtr->StopCount(); ++i) {
        auto stop = busSystemPtr->StopByIndex(i);
        if (stop)
            sortedStopIDs.push_back(stop->ID());
    }
    std::sort(sortedStopIDs.begin(), sortedStopIDs.end());
    if (idx < sortedStopIDs.size()) {
        return busSystemPtr->StopByID(sortedStopIDs[idx]);
    }
    return nullptr;
}

/*std::shared_ptr<CBusSystem::SRoute> CBusSystemIndexer::SImplementation::SortedRouteByIndex(std::size_t idx) const {
    if (busSystemPtr->RouteCount() < idx) {
        return nullptr;
    }
    std::vector<std::string> routeNames;
    for (std::size_t i = 0; i < busSystemPtr->RouteCount(); ++i) {
        auto route = busSystemPtr->RouteByIndex(i);
        if (route)
            routeNames.push_back(route->Name());
    }
    std::sort(routeNames.begin(), routeNames.end());
    if (idx < routeNames.size())
        return busSystemPtr->RouteByName(routeNames[idx]);
    return nullptr;
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SImplementation::StopByNodeID(TNodeID nodeId) const {
    std::shared_ptr<CBusSystem::SStop> output = nullptr;
    for (std::size_t i = 0; i < busSystemPtr->StopCount(); ++i) {
        auto stop = busSystemPtr->StopByIndex(i);
        if (stop && stop->NodeID() == nodeId) {
            output = stop;
        }
    }
    return output;
}

bool CBusSystemIndexer::SImplementation::RoutesByNodeIDs(TNodeID src, TNodeID dest,
                         std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> &outRoutes) const {
    auto sourceStop = StopByNodeID(src);
    auto destStop = StopByNodeID(dest);
    if (!sourceStop || !destStop) {
        return false;
    }

    CBusSystem::TStopID srcID = sourceStop->ID();
    CBusSystem::TStopID destID = destStop->ID();

    for (std::size_t i = 0; i < busSystemPtr->RouteCount(); ++i) {
        auto route = busSystemPtr->RouteByIndex(i);
        if (route) {
            bool hasSrc = false, hasDest = false;
            for (std::size_t j = 0; j < route->StopCount(); ++j) {
                if (route->GetStopID(j) == srcID)
                    hasSrc = true;
                if (route->GetStopID(j) == destID)
                    hasDest = true;
                if (hasSrc && hasDest) {
                    outRoutes.insert(route);
                    break;
                }
            }
        }
    }
    return outRoutes.size() > 0;
}

bool CBusSystemIndexer::SImplementation::RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const {
    std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> tempRoutes;
    return RoutesByNodeIDs(src, dest, tempRoutes);
}
*/