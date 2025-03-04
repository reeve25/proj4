#include "OpenStreetMap.h"    // header for COpenStreetMap class
#include "XMLReader.h"        // header for XML parsing functionality
#include <memory>             // for smart pointers like std::shared_ptr and std::unique_ptr
#include <vector>             // dynamic arrays for storing nodes and ways
#include <string>             // for string handling
#include <unordered_map>      // for fast attribute lookup
#include <iterator>           // for std::advance

// Define the implementation structure with internal classes.
struct COpenStreetMap::SImplementation {
    // Forward declarations of the internal classes.
    class SNodeImpl;
    class SWayImpl;
    // Containers for nodes and ways.
    std::vector<std::shared_ptr<SNodeImpl>> nodesCollection;
    std::vector<std::shared_ptr<SWayImpl>> waysCollection;
};

// Implementation of the node class, inheriting from CStreetMap::SNode.
class COpenStreetMap::SImplementation::SNodeImpl : public CStreetMap::SNode {
public:
    TNodeID nodeIdentifier;  // Unique node ID
    TLocation coordinates;   // Latitude and longitude
    std::unordered_map<std::string, std::string> attrMap; // Attributes for the node

    TNodeID ID() const noexcept override {
        return nodeIdentifier;
    }

    TLocation Location() const noexcept override {
        return coordinates;
    }

    std::size_t AttributeCount() const noexcept override {
        return attrMap.size();
    }

    std::string GetAttributeKey(std::size_t pos) const noexcept override {
        if (pos < attrMap.size()) {
            auto it = attrMap.begin();
            std::advance(it, pos);
            return it->first;
        }
        return "";
    }

    bool HasAttribute(const std::string &key) const noexcept override {
        return attrMap.find(key) != attrMap.end();
    }

    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = attrMap.find(key);
        if (it != attrMap.end()) {
            return it->second;
        }
        return "";
    }
};

// Implementation of the way class, inheriting from CStreetMap::SWay.
class COpenStreetMap::SImplementation::SWayImpl : public CStreetMap::SWay {
public:
    TWayID wayIdentifier;  // Unique way ID
    std::vector<TNodeID> nodeRefs;  // List of node IDs forming the way
    std::unordered_map<std::string, std::string> attrMap; // Attributes for the way

    TWayID ID() const noexcept override {
        return wayIdentifier;
    }

    std::size_t NodeCount() const noexcept override {
        return nodeRefs.size();
    }

    TNodeID GetNodeID(std::size_t pos) const noexcept override {
        if (pos < nodeRefs.size()) {
            return nodeRefs[pos];
        }
        return CStreetMap::InvalidNodeID;
    }

    std::size_t AttributeCount() const noexcept override {
        return attrMap.size();
    }

    std::string GetAttributeKey(std::size_t pos) const noexcept override {
        if (pos < attrMap.size()) {
            auto it = attrMap.begin();
            std::advance(it, pos);
            return it->first;
        }
        return "";
    }

    bool HasAttribute(const std::string &key) const noexcept override {
        return attrMap.find(key) != attrMap.end();
    }

    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = attrMap.find(key);
        if (it != attrMap.end()) {
            return it->second;
        }
        return "";
    }
};

// Constructor: Reads XML entities and constructs nodes and ways.
COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) {
    DImplementation = std::make_unique<SImplementation>();

    SXMLEntity xmlEntity;
    // Use separate pointers for current node and way.
    std::shared_ptr<SImplementation::SNodeImpl> curNode = nullptr;
    std::shared_ptr<SImplementation::SWayImpl> curWay = nullptr;

    while (src->ReadEntity(xmlEntity)) {
        if (xmlEntity.DType == SXMLEntity::EType::StartElement) {
            if (xmlEntity.DNameData == "node") {
                curNode = std::make_shared<SImplementation::SNodeImpl>();
                curWay = nullptr;
                for (const auto &attribute : xmlEntity.DAttributes) {
                    const std::string &attrName = attribute.first;
                    const std::string &attrValue = attribute.second;
                    if (attrName == "id") {
                        curNode->nodeIdentifier = std::stoull(attrValue);
                    } else if (attrName == "lat") {
                        curNode->coordinates.first = std::stod(attrValue);
                    } else if (attrName == "lon") {
                        curNode->coordinates.second = std::stod(attrValue);
                    } else {
                        curNode->attrMap[attrName] = attrValue;
                    }
                }
            } else if (xmlEntity.DNameData == "way") {
                curWay = std::make_shared<SImplementation::SWayImpl>();
                curNode = nullptr;
                for (const auto &attribute : xmlEntity.DAttributes) {
                    if (attribute.first == "id") {
                        curWay->wayIdentifier = std::stoull(attribute.second);
                    } else {
                        curWay->attrMap[attribute.first] = attribute.second;
                    }
                }
            } else if (xmlEntity.DNameData == "nd" && curWay) {
                for (const auto &attribute : xmlEntity.DAttributes) {
                    if (attribute.first == "ref") {
                        curWay->nodeRefs.push_back(std::stoull(attribute.second));
                    }
                }
            } else if (xmlEntity.DNameData == "tag") {
                std::string key, value;
                for (const auto &attribute : xmlEntity.DAttributes) {
                    if (attribute.first == "k") {
                        key = attribute.second;
                    } else if (attribute.first == "v") {
                        value = attribute.second;
                    }
                }
                if (!key.empty()) {
                    if (curNode) {
                        curNode->attrMap[key] = value;
                    } else if (curWay) {
                        curWay->attrMap[key] = value;
                    }
                }
            }
        } else if (xmlEntity.DType == SXMLEntity::EType::EndElement) {
            if (xmlEntity.DNameData == "node" && curNode) {
                DImplementation->nodesCollection.push_back(curNode);
                curNode = nullptr;
            } else if (xmlEntity.DNameData == "way" && curWay) {
                DImplementation->waysCollection.push_back(curWay);
                curWay = nullptr;
            }
        }
    }
}

COpenStreetMap::~COpenStreetMap() = default;

std::size_t COpenStreetMap::NodeCount() const noexcept {
    return DImplementation->nodesCollection.size();
}

std::size_t COpenStreetMap::WayCount() const noexcept {
    return DImplementation->waysCollection.size();
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t idx) const noexcept {
    if (idx < DImplementation->nodesCollection.size())
        return DImplementation->nodesCollection[idx];
    return nullptr;
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept {
    for (const auto &node : DImplementation->nodesCollection) {
        if (node->ID() == id)
            return node;
    }
    return nullptr;
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t idx) const noexcept {
    if (idx < DImplementation->waysCollection.size())
        return DImplementation->waysCollection[idx];
    return nullptr;
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept {
    for (const auto &way : DImplementation->waysCollection) {
        if (way->ID() == id)
            return way;
    }
    return nullptr;
}
