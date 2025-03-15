#include "DijkstraTransportationPlanner.h"
#include "DijkstraPathRouter.h"
#include "GeographicUtils.h"
#include <queue>
#include <unordered_map>
#include <set>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <limits>

struct CDijkstraTransportationPlanner::SImplementation {
    std::shared_ptr<SConfiguration> configPtr;
    std::vector<std::shared_ptr<CStreetMap::SNode>> orderedNodes;
    std::shared_ptr<CDijkstraPathRouter> distRouter;
    std::shared_ptr<CDijkstraPathRouter> timeRouter;
    std::unordered_map<CStreetMap::TNodeID, CPathRouter::TVertexID> nodeToDistVertex;
    std::unordered_map<CStreetMap::TNodeID, CPathRouter::TVertexID> nodeToTimeVertex;
    std::unordered_map<CPathRouter::TVertexID, CStreetMap::TNodeID> distVertexToNode;
    std::unordered_map<CPathRouter::TVertexID, CStreetMap::TNodeID> timeVertexToNode;
    std::unordered_map<CStreetMap::TNodeID, size_t> nodeIndexMap;
    std::unordered_map<CBusSystem::TStopID, CStreetMap::TNodeID> stopToNodeMap;
    std::unordered_map<CBusSystem::TStopID, std::string> stopNames;
    std::unordered_map<CStreetMap::TNodeID, CBusSystem::TStopID> nodeToStop;
    std::unordered_map<CStreetMap::TNodeID, std::set<std::pair<std::string, CStreetMap::TNodeID>>> busRoutes;
    
    SImplementation(std::shared_ptr<SConfiguration> cfg)
        : configPtr(cfg) {
        auto streetMap = configPtr->StreetMap();
        auto busSystem = configPtr->BusSystem();
        
        // Create path routers.
        distRouter = std::make_shared<CDijkstraPathRouter>();
        timeRouter = std::make_shared<CDijkstraPathRouter>();
        
        // Build and sort nodes.
        for (size_t i = 0; i < streetMap->NodeCount(); ++i) {
            auto node = streetMap->NodeByIndex(i);
            orderedNodes.push_back(node);
        }
        std::sort(orderedNodes.begin(), orderedNodes.end(),
            [](const auto &a, const auto &b) {
                return a->ID() < b->ID();
            });
        for (size_t i = 0; i < orderedNodes.size(); ++i) {
            nodeIndexMap[orderedNodes[i]->ID()] = i;
        }
        
        // Build vertex mappings and add vertices to both routers.
        for (size_t i = 0; i < orderedNodes.size(); ++i) {
            auto node = orderedNodes[i];
            auto dVert = distRouter->AddVertex(node->ID());
            auto tVert = timeRouter->AddVertex(node->ID());
            nodeToDistVertex[node->ID()] = dVert;
            nodeToTimeVertex[node->ID()] = tVert;
            distVertexToNode[dVert] = node->ID();
            timeVertexToNode[tVert] = node->ID();
        }
        
        // Map bus stops to nodes.
        for (size_t i = 0; i < busSystem->StopCount(); ++i) {
            auto stop = busSystem->StopByIndex(i);
            stopToNodeMap[stop->ID()] = stop->NodeID();
            auto nodeID = stop->NodeID();
            if (nodeToStop.find(nodeID) == nodeToStop.end() || stop->ID() < nodeToStop[nodeID])
                nodeToStop[nodeID] = stop->ID();
        }
        
        // Build bus route information.
        for (size_t r = 0; r < busSystem->RouteCount(); ++r) {
            auto route = busSystem->RouteByIndex(r);
            std::string routeName = route->Name();
            for (size_t i = 0; i < route->StopCount() - 1; ++i) {
                auto currStopID = route->GetStopID(i);
                auto nextStopID = route->GetStopID(i + 1);
                auto currStop = busSystem->StopByID(currStopID);
                auto nextStop = busSystem->StopByID(nextStopID);
                if (currStop && nextStop) {
                    auto currNodeID = currStop->NodeID();
                    auto nextNodeID = nextStop->NodeID();
                    busRoutes[currNodeID].insert({routeName, nextNodeID});
                }
            }
        }
        
        // Process multi-node ways (node count > 2).
        for (size_t i = 0; i < streetMap->WayCount(); ++i) {
            auto way = streetMap->WayByIndex(i);
            if (way->NodeCount() <= 2)
                continue;
            bool isOneway = false;
            if (way->HasAttribute("oneway")) {
                std::string val = way->GetAttribute("oneway");
                isOneway = (val == "yes" || val == "true" || val == "1");
            }
            for (size_t j = 1; j < way->NodeCount(); ++j) {
                auto srcID = way->GetNodeID(j - 1);
                auto destID = way->GetNodeID(j);
                if (srcID == CStreetMap::InvalidNodeID || destID == CStreetMap::InvalidNodeID)
                    continue;
                auto srcNode = streetMap->NodeByID(srcID);
                auto destNode = streetMap->NodeByID(destID);
                if (!srcNode || !destNode)
                    continue;
                double dist = SGeographicUtils::HaversineDistanceInMiles(srcNode->Location(), destNode->Location());
                if (dist <= 0.0)
                    continue;
                auto srcDVert = nodeToDistVertex[srcID];
                auto destDVert = nodeToDistVertex[destID];
                distRouter->AddEdge(srcDVert, destDVert, dist, false);
                if (!isOneway)
                    distRouter->AddEdge(destDVert, srcDVert, dist, false);
                
                auto srcTVert = nodeToTimeVertex[srcID];
                auto destTVert = nodeToTimeVertex[destID];
                double walkTime = dist / configPtr->WalkSpeed();
                timeRouter->AddEdge(srcTVert, destTVert, walkTime, false);
                timeRouter->AddEdge(destTVert, srcTVert, walkTime, false);
                double bikeTime = dist / configPtr->BikeSpeed();
                timeRouter->AddEdge(srcTVert, destTVert, bikeTime, false);
                if (!isOneway)
                    timeRouter->AddEdge(destTVert, srcTVert, bikeTime, false);
                double speedLimit = configPtr->DefaultSpeedLimit();
                if (way->HasAttribute("maxspeed")) {
                    try {
                        std::string spd = way->GetAttribute("maxspeed");
                        size_t pos = spd.find(' ');
                        if (pos != std::string::npos)
                            spd = spd.substr(0, pos);
                        speedLimit = std::stod(spd);
                    } catch (...) { }
                }
                double driveTime = dist / speedLimit;
                timeRouter->AddEdge(srcTVert, destTVert, driveTime, false);
                if (!isOneway)
                    timeRouter->AddEdge(destTVert, srcTVert, driveTime, false);
            }
        }
        
        // Process direct ways (node count == 2).
        for (size_t i = 0; i < streetMap->WayCount(); ++i) {
            auto way = streetMap->WayByIndex(i);
            if (way->NodeCount() != 2)
                continue;
            bool isOneway = false;
            if (way->HasAttribute("oneway")) {
                std::string val = way->GetAttribute("oneway");
                isOneway = (val == "yes" || val == "true" || val == "1");
            }
            auto srcID = way->GetNodeID(0);
            auto destID = way->GetNodeID(1);
            if (srcID == CStreetMap::InvalidNodeID || destID == CStreetMap::InvalidNodeID)
                continue;
            auto srcNode = streetMap->NodeByID(srcID);
            auto destNode = streetMap->NodeByID(destID);
            if (!srcNode || !destNode)
                continue;
            double dist = SGeographicUtils::HaversineDistanceInMiles(srcNode->Location(), destNode->Location());
            if (dist <= 0.0)
                continue;
            auto srcDVert = nodeToDistVertex[srcID];
            auto destDVert = nodeToDistVertex[destID];
            distRouter->AddEdge(srcDVert, destDVert, dist, false);
            if (!isOneway)
                distRouter->AddEdge(destDVert, srcDVert, dist, false);
            
            auto srcTVert = nodeToTimeVertex[srcID];
            auto destTVert = nodeToTimeVertex[destID];
            double walkTime = dist / configPtr->WalkSpeed();
            timeRouter->AddEdge(srcTVert, destTVert, walkTime, false);
            timeRouter->AddEdge(destTVert, srcTVert, walkTime, false);
            double bikeTime = dist / configPtr->BikeSpeed();
            timeRouter->AddEdge(srcTVert, destTVert, bikeTime, false);
            if (!isOneway)
                timeRouter->AddEdge(destTVert, srcTVert, bikeTime, false);
            double speedLimit = configPtr->DefaultSpeedLimit();
            if (way->HasAttribute("maxspeed")) {
                try {
                    std::string spd = way->GetAttribute("maxspeed");
                    size_t pos = spd.find(' ');
                    if (pos != std::string::npos)
                        spd = spd.substr(0, pos);
                    speedLimit = std::stod(spd);
                } catch (...) { }
            }
            double driveTime = dist / speedLimit;
            timeRouter->AddEdge(srcTVert, destTVert, driveTime, false);
            if (!isOneway)
                timeRouter->AddEdge(destTVert, srcTVert, driveTime, false);
        }
        
        // Add bus route edges to the time router.
        for (const auto &entry : busRoutes) {
            CStreetMap::TNodeID nodeID = entry.first;
            for (const auto &routePair : entry.second) {
                // Route name is not used here.
                CStreetMap::TNodeID nextNodeID = routePair.second;
                auto srcNode = streetMap->NodeByID(nodeID);
                auto destNode = streetMap->NodeByID(nextNodeID);
                if (!srcNode || !destNode)
                    continue;
                double dist = SGeographicUtils::HaversineDistanceInMiles(srcNode->Location(), destNode->Location());
                double busTime = dist / configPtr->DefaultSpeedLimit() + (configPtr->BusStopTime() / 3600.0);
                auto srcTVert = nodeToTimeVertex[nodeID];
                auto destTVert = nodeToTimeVertex[nextNodeID];
                timeRouter->AddEdge(srcTVert, destTVert, busTime, false);
            }
        }
    }
    
    std::string FindBusRouteBetweenNodes(const CStreetMap::TNodeID& src,
                                          const CStreetMap::TNodeID& dest) const {
        if (busRoutes.count(src) == 0)
            return "";
        std::vector<std::string> directRoutes;
        for (const auto &entry : busRoutes.at(src)) {
            if (entry.second == dest)
                directRoutes.push_back(entry.first);
        }
        if (!directRoutes.empty()) {
            std::sort(directRoutes.begin(), directRoutes.end());
            return directRoutes.front();
        }
        return "";
    }
};

CDijkstraTransportationPlanner::CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config)
    : DImplementation(std::make_unique<SImplementation>(config)) {
}

CDijkstraTransportationPlanner::~CDijkstraTransportationPlanner() = default;

std::size_t CDijkstraTransportationPlanner::NodeCount() const noexcept {
    return DImplementation->orderedNodes.size();
}

std::shared_ptr<CStreetMap::SNode> CDijkstraTransportationPlanner::SortedNodeByIndex(std::size_t index) const noexcept {
    if (index < DImplementation->orderedNodes.size())
        return DImplementation->orderedNodes[index];
    return nullptr;
}

double CDijkstraTransportationPlanner::FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) {
    path.clear();
    if (DImplementation->nodeToDistVertex.find(src) == DImplementation->nodeToDistVertex.end() ||
        DImplementation->nodeToDistVertex.find(dest) == DImplementation->nodeToDistVertex.end())
        return CPathRouter::NoPathExists;
    
    auto srcVertex = DImplementation->nodeToDistVertex[src];
    auto destVertex = DImplementation->nodeToDistVertex[dest];
    std::vector<CPathRouter::TVertexID> routerPath;
    double distance = DImplementation->distRouter->FindShortestPath(srcVertex, destVertex, routerPath);
    if (distance < 0.0)
        return CPathRouter::NoPathExists;
    
    for (const auto &vID : routerPath)
        path.push_back(DImplementation->distVertexToNode[vID]);
    return distance;
}

double CDijkstraTransportationPlanner::FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) {
    path.clear();
    if (src == dest) {
        path.push_back({ETransportationMode::Walk, src});
        return 0.0;
    }
    if (DImplementation->nodeToDistVertex.find(src) == DImplementation->nodeToDistVertex.end() ||
        DImplementation->nodeToDistVertex.find(dest) == DImplementation->nodeToDistVertex.end())
        return CPathRouter::NoPathExists;
    
    auto srcVertex = DImplementation->nodeToTimeVertex[src];
    auto destVertex = DImplementation->nodeToTimeVertex[dest];
    std::vector<CPathRouter::TVertexID> routerPath;
    double totalTime = DImplementation->timeRouter->FindShortestPath(srcVertex, destVertex, routerPath);
    if (totalTime < 0.0 || routerPath.empty())
        return CPathRouter::NoPathExists;
    
    std::vector<TNodeID> nodeSequence;
    for (const auto &vertex : routerPath)
        nodeSequence.push_back(DImplementation->timeVertexToNode[vertex]);
    
    // Dynamically build the trip path.
    // For this version, if a bus route exists between two nodes, we force Bus mode.
    ETransportationMode lastMode = ETransportationMode::Walk;
    std::string currentBusRoute = "";
    path.push_back({lastMode, nodeSequence[0]});
    
    for (size_t i = 1; i < nodeSequence.size(); ++i) {
        auto prevNode = DImplementation->configPtr->StreetMap()->NodeByID(nodeSequence[i-1]);
        auto currNode = DImplementation->configPtr->StreetMap()->NodeByID(nodeSequence[i]);
        if (!prevNode || !currNode)
            continue;
        
        double segDistance = SGeographicUtils::HaversineDistanceInMiles(prevNode->Location(), currNode->Location());
        double walkDuration = segDistance / DImplementation->configPtr->WalkSpeed();
        double bikeDuration = segDistance / DImplementation->configPtr->BikeSpeed();
        
        std::string busOption = DImplementation->FindBusRouteBetweenNodes(nodeSequence[i-1], nodeSequence[i]);
        // Force Bus mode if a bus option exists.
        ETransportationMode chosenMode;
        if (!busOption.empty()) {
            chosenMode = ETransportationMode::Bus;
            currentBusRoute = busOption;
        } else if (bikeDuration < walkDuration) {
            chosenMode = ETransportationMode::Bike;
            currentBusRoute = "";
        } else {
            chosenMode = ETransportationMode::Walk;
            currentBusRoute = "";
        }
        
        if (chosenMode == lastMode)
            path.back().second = nodeSequence[i];
        else
            path.push_back({chosenMode, nodeSequence[i]});
        
        lastMode = chosenMode;
    }
    
    return totalTime;
}

bool CDijkstraTransportationPlanner::GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const {
    return true;
}
