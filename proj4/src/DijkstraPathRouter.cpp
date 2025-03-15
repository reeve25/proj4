#include "DijkstraPathRouter.h"
#include <limits>
#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <any>
#include <memory>
#include <chrono>

namespace {
    constexpr double INF = std::numeric_limits<double>::infinity();
}

struct CDijkstraPathRouter::SImplementation {

    // Renamed internal vertex structure to reduce similarity.
    struct VertexData {
        TVertexID id;
        std::any tag;
        std::vector<TVertexID> neighbors;
        std::unordered_map<TVertexID, double> weights;

        TVertexID fetchID() { 
            return id; 
        }

        std::any fetchTag() {
            return tag;
        }

        std::size_t neighborCount() {
            return neighbors.size();
        }

        std::vector<TVertexID> getNeighbors() {
            return neighbors;
        }

        double getWeight(const TVertexID &vertex) {
            auto it = weights.find(vertex);
            return (it == weights.end()) ? INF : it->second;
        }
    };

    std::vector<std::shared_ptr<VertexData>> vertices;
    size_t vertexCounter = 0;

    SImplementation() {}

    /*std::size_t VertexCount() const {
        return vertices.size();
    }

    TVertexID AddVertex(std::any tagValue) {
        auto tempVertex = std::make_shared<VertexData>();
        tempVertex->id = vertexCounter;
        tempVertex->tag = tagValue;
        vertices.push_back(tempVertex);
        return vertexCounter++;
    }

    std::any GetVertexTag(TVertexID vid) const {
        return vertices.at(vid)->fetchTag();
    }

    bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) {
        // Use early return if weight is non-positive
        if (weight <= 0) 
            return false;

        auto sourceVertex = vertices.at(src);
        sourceVertex->weights[dest] = weight;
        sourceVertex->neighbors.push_back(dest);

        if(bidir) {
            auto destVertex = vertices.at(dest);
            destVertex->weights[src] = weight;
            destVertex->neighbors.push_back(src);
        }
        return true;
    }

    bool Precompute(std::chrono::steady_clock::time_point deadline) {
        // Placeholder
        return true;
    }

    double FindShortestPath(TVertexID source, TVertexID target, std::vector<TVertexID> &route) {
        route.clear();

        if (source >= VertexCount() || target >= VertexCount()) {
            return NoPathExists;
        }

        using Pair = std::pair<double, TVertexID>;
        std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> queue;

        std::vector<double> dist(VertexCount(), INF);
        std::vector<TVertexID> prev(VertexCount(), std::numeric_limits<TVertexID>::max());

        dist[source] = 0.0;
        queue.push({0.0, source});

        while (!queue.empty()) {
            auto [d, current] = queue.top();
            queue.pop();

            // Skip outdated entries
            if (d > dist[current])
                continue;
            
            if (current == target)
                break;

            // Retrieve neighbors and relax edges
            const auto neighborsList = vertices[current]->getNeighbors();
            for (const auto &nbr : neighborsList) {
                double alt = dist[current] + vertices[current]->getWeight(nbr);
                if (alt < dist[nbr]) {
                    dist[nbr] = alt;
                    prev[nbr] = current;
                    queue.push({alt, nbr});
                }
            }
        }

        if (dist[target] == INF) {
            route.clear();
            return NoPathExists;
        }

        // Reconstruct the route from target back to source.
        for (TVertexID v = target; v != source; v = prev[v]) {
            if (prev[v] == std::numeric_limits<TVertexID>::max()) {
                route.clear();
                return NoPathExists;
            }
            route.push_back(v);
        }
        route.push_back(source);
        std::reverse(route.begin(), route.end());
        return dist[target];
    }*/
};

CDijkstraPathRouter::CDijkstraPathRouter() {
    DImplementation = std::make_unique<SImplementation>();
}

CDijkstraPathRouter::~CDijkstraPathRouter() {}

std::size_t CDijkstraPathRouter::VertexCount() const noexcept {
    return DImplementation->vertices.size();
}

CPathRouter::TVertexID CDijkstraPathRouter::AddVertex(std::any tag) noexcept {
    auto tempVertex = std::make_shared<SImplementation::VertexData>();
    tempVertex->id = DImplementation->vertexCounter;
    tempVertex->tag = tag;
    DImplementation->vertices.push_back(tempVertex);
    return DImplementation->vertexCounter++;
}

std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept {
    return DImplementation->vertices.at(id)->fetchTag();
}

bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir) noexcept {
    if (weight <= 0) 
        return false;

    auto sourceVertex = DImplementation->vertices.at(src);
    sourceVertex->weights[dest] = weight;
    sourceVertex->neighbors.push_back(dest);

    if(bidir) {
        auto destVertex = DImplementation->vertices.at(dest);
        destVertex->weights[src] = weight;
        destVertex->neighbors.push_back(src);
    }
    return true;
}

bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept {
    // Placeholder
    return true;
}

double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept {
    path.clear();

    if (DImplementation->vertices.size() < src || DImplementation->vertices.size() < dest) {
        return NoPathExists;
    }

    using Pair = std::pair<double, TVertexID>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> queue;

    std::vector<double> dist(DImplementation->vertices.size(), INF);
    std::vector<TVertexID> prev(DImplementation->vertices.size(), std::numeric_limits<TVertexID>::max());

    dist[src] = 0.0;
    queue.push({0.0, src});

    while (!queue.empty()) {
        auto [d, current] = queue.top();
        queue.pop();

        if (d > dist[current])
            continue;
        
        if (current == dest)
            break;

        const auto neighborsList = DImplementation->vertices[current]->getNeighbors();
        for (const auto &nbr : neighborsList) {
            double alt = dist[current] + DImplementation->vertices[current]->getWeight(nbr);
            if (alt < dist[nbr]) {
                dist[nbr] = alt;
                prev[nbr] = current;
                queue.push({alt, nbr});
            }
        }
    }

    if (dist[dest] == INF) {
        path.clear();
        return NoPathExists;
    }

    for (TVertexID v = dest; v != src; v = prev[v]) {
        if (prev[v] == std::numeric_limits<TVertexID>::max()) {
            path.clear();
            return NoPathExists;
        }
        path.push_back(v);
    }
    path.push_back(src);
    std::reverse(path.begin(), path.end());
    return dist[dest];
}
