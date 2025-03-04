#include "OpenStreetMap.h"   // include openstreetmap header
#include "XMLReader.h"       // include xmlreader header for parsing xml
#include <memory>            // for smart pointers
#include <vector>            // for dynamic arrays
#include <string>            // for text handling
#include <unordered_map>     // for attribute lookups
#include <iterator>          // for std::next

// helper functions to parse attributes for nodes and ways
namespace {
   // fills in node attributes from the xml attributes vector
   void fillNodeAttributes(const std::vector<std::pair<std::string, std::string>> &attrs,
                           std::shared_ptr<COpenStreetMap::SImplementation::SNodeImpl> &node)
   {
      for (const auto &attr : attrs) {
         if (attr.first == "id") {
            node->nodeIdentifier = std::stoull(attr.second);
         }
         else if (attr.first == "lat") {
            node->coordinates.first = std::stod(attr.second);
         }
         else if (attr.first == "lon") {
            node->coordinates.second = std::stod(attr.second);
         }
         else {
            node->Attributes[attr.first] = attr.second;
         }
      }
   }

   // fills in way attributes from the xml attributes vector
   void fillWayAttributes(const std::vector<std::pair<std::string, std::string>> &attrs,
                          std::shared_ptr<COpenStreetMap::SImplementation::SWayImpl> &way)
   {
      for (const auto &attr : attrs) {
         if (attr.first == "id") {
            way->WayID = std::stoull(attr.second);
         }
         else {
            way->Attributes[attr.first] = attr.second;
         }
      }
   }
}

// internal implementation structure with nested node and way classes
struct COpenStreetMap::SImplementation {
    class SNodeImpl;
    class SWayImpl;
    std::vector<std::shared_ptr<SNodeImpl>> Nodes;
    std::vector<std::shared_ptr<SWayImpl>> Ways;
};

// node implementation inheriting from CStreetMap::SNode
class COpenStreetMap::SImplementation::SNodeImpl : public CStreetMap::SNode {
public:
    TNodeID nodeIdentifier;
    TLocation coordinates;
    std::unordered_map<std::string, std::string> Attributes;

    TNodeID ID() const noexcept override {
        return nodeIdentifier;
    }
    TLocation Location() const noexcept override {
        return coordinates;
    }
    std::size_t AttributeCount() const noexcept override {
        return Attributes.size();
    }
    std::string GetAttributeKey(std::size_t index) const noexcept override {
        if (index < Attributes.size()) {
            auto it = std::next(Attributes.begin(), index);
            return it->first;
        }
        return "";
    }
    bool HasAttribute(const std::string &key) const noexcept override {
        return Attributes.find(key) != Attributes.end();
    }
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = Attributes.find(key);
        return (it != Attributes.end()) ? it->second : "";
    }
};

// way implementation inheriting from CStreetMap::SWay
class COpenStreetMap::SImplementation::SWayImpl : public CStreetMap::SWay {
public:
    TWayID WayID;
    std::vector<TNodeID> NodeIDs;
    std::unordered_map<std::string, std::string> Attributes;

    TWayID ID() const noexcept override {
        return WayID;
    }
    std::size_t NodeCount() const noexcept override {
        return NodeIDs.size();
    }
    TNodeID GetNodeID(std::size_t index) const noexcept override {
        return index < NodeIDs.size() ? NodeIDs[index] : CStreetMap::InvalidNodeID;
    }
    std::size_t AttributeCount() const noexcept override {
        return Attributes.size();
    }
    std::string GetAttributeKey(std::size_t index) const noexcept override {
        if (index < Attributes.size()) {
            auto it = std::next(Attributes.begin(), index);
            return it->first;
        }
        return "";
    }
    bool HasAttribute(const std::string &key) const noexcept override {
        return Attributes.find(key) != Attributes.end();
    }
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = Attributes.find(key);
        return (it != Attributes.end()) ? it->second : "";
    }
};

COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) {
    DImplementation = std::make_unique<SImplementation>();
    SXMLEntity entity;
    std::shared_ptr<SImplementation::SNodeImpl> currentNode = nullptr;
    std::shared_ptr<SImplementation::SWayImpl> currentWay = nullptr;

    while (src->ReadEntity(entity)) {
        if (entity.DType == SXMLEntity::EType::StartElement) {
            if (entity.DNameData == "node") {
                currentNode = std::make_shared<SImplementation::SNodeImpl>();
                currentWay.reset();
                fillNodeAttributes(entity.DAttributes, currentNode);
            }
            else if (entity.DNameData == "way") {
                currentWay = std::make_shared<SImplementation::SWayImpl>();
                currentNode.reset();
                fillWayAttributes(entity.DAttributes, currentWay);
            }
            else if (entity.DNameData == "nd" && currentWay) {
                for (const auto &attr : entity.DAttributes) {
                    if (attr.first == "ref") {
                        currentWay->NodeIDs.push_back(std::stoull(attr.second));
                    }
                }
            }
            else if (entity.DNameData == "tag") {
                std::string key, value;
                for (const auto &attr : entity.DAttributes) {
                    if (attr.first == "k")
                        key = attr.second;
                    else if (attr.first == "v")
                        value = attr.second;
                }
                if (!key.empty()) {
                    if (currentNode)
                        currentNode->Attributes[key] = value;
                    else if (currentWay)
                        currentWay->Attributes[key] = value;
                }
            }
        }
        else if (entity.DType == SXMLEntity::EType::EndElement) {
            if (entity.DNameData == "node" && currentNode) {
                DImplementation->Nodes.push_back(currentNode);
                currentNode.reset();
            }
            else if (entity.DNameData == "way" && currentWay) {
                DImplementation->Ways.push_back(currentWay);
                currentWay.reset();
            }
        }
    }
}

COpenStreetMap::~COpenStreetMap() = default;

std::size_t COpenStreetMap::NodeCount() const noexcept {
    return DImplementation->Nodes.size();
}

std::size_t COpenStreetMap::WayCount() const noexcept {
    return DImplementation->Ways.size();
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept {
    return index < DImplementation->Nodes.size() ? DImplementation->Nodes[index] : nullptr;
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept {
    for (const auto &node : DImplementation->Nodes) {
        if (node->ID() == id)
            return node;
    }
    return nullptr;
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept {
    return index < DImplementation->Ways.size() ? DImplementation->Ways[index] : nullptr;
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept {
    for (const auto &way : DImplementation->Ways) {
        if (way->ID() == id)
            return way;
    }
    return nullptr;
}
