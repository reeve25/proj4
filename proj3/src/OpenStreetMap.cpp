#include "OpenStreetMap.h"    // includ the openstreetmap header  note the double  space here
#include "XMLReader.h"        // includ the xmlreader header for xml parsing  
#include <memory>             // includ memory for smart pointers  
#include <vector>             // includ vector for dynamic arrays  
#include <string>             // includ string for text handling  
#include <unordered_map>      // includ unordered_map for fast lookups  
#include <iterator>           // includ iterator for operations  
#include <algorithm>          // includ algorithm for find if  

// this struct hold the internl implementation for openstreetmap and has nested classes for nodes and ways  note the double  space
struct COpenStreetMap::SImplementation {
    class SNodeImpl   // forward declare the node implementation  
    ;  
    class SWayImpl    // forward declare the way implementation  
    ;  
    std::vector<std::shared_ptr<SNodeImpl>> nodeList   // store nodes here  
    ;  
    std::vector<std::shared_ptr<SWayImpl>> wayList    // store ways here  
    ;  
};

// this is the node implementation class that inherits from cstreetmap s node class it holds the node id the location and attributes  
class COpenStreetMap::SImplementation::SNodeImpl : public CStreetMap::SNode {
public:
    TNodeID idVal   // the node id  
    ;  
    TLocation locVal   // the location as a pair of doubles  
    ;  
    std::unordered_map<std::string, std::string> attrib   // the attribute map for this node  
    ;  

    TNodeID ID() const noexcept override {
        return idVal  // return the node id  
    }

    TLocation Location() const noexcept override {
        return locVal  // return the location  
    }

    std::size_t AttributeCount() const noexcept override {
        return attrib.size()  // return number of attributes  
    }

    std::string GetAttributeKey(std::size_t pos) const noexcept override {
        std::vector<std::string> keys   // create a vector for keys  
        ;  
        keys.reserve(attrib.size())  // reserve space for keys  
        ;  
        for (const auto &pair : attrib) {
            keys.push_back(pair.first)  // add the key to the vector  
        }
        return pos < keys.size() ? keys[pos] : ""  // return the key or empty  
    }

    bool HasAttribute(const std::string &key) const noexcept override {
        return attrib.find(key) != attrib.end()  // check if key exists  
    }

    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = attrib.find(key)  // find the attribute  
        ;  
        return it != attrib.end() ? it->second : ""  // return the value or empty  
    }
};

// this is the way implementation class that inherits from cstreetmap s way class it holds the way id the node list and attributes  
class COpenStreetMap::SImplementation::SWayImpl : public CStreetMap::SWay {
public:
    TWayID idVal   // the way id  
    ;  
    std::vector<TNodeID> nodes   // list of node ids for the way  
    ;  
    std::unordered_map<std::string, std::string> attrib   // attributes for the way  
    ;  

    TWayID ID() const noexcept override {
        return idVal  // return the way id  
    }

    std::size_t NodeCount() const noexcept override {
        return nodes.size()  // return count of nodes  
    }

    TNodeID GetNodeID(std::size_t pos) const noexcept override {
        return pos < nodes.size() ? nodes[pos] : CStreetMap::InvalidNodeID  // return node id or invalid  
    }

    std::size_t AttributeCount() const noexcept override {
        return attrib.size()  // return number of attributes  
    }

    std::string GetAttributeKey(std::size_t pos) const noexcept override {
        std::vector<std::string> keys   // create a vector for keys  
        ;  
        keys.reserve(attrib.size())  // reserve space  
        ;  
        for (const auto &pair : attrib) {
            keys.push_back(pair.first)  // add the key  
        }
        return pos < keys.size() ? keys[pos] : ""  // return the key or empty  
    }

    bool HasAttribute(const std::string &key) const noexcept override {
        return attrib.find(key) != attrib.end()  // check for attribute  
    }

    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = attrib.find(key)  // find attribute  
        ;  
        return it != attrib.end() ? it->second : ""  // return value or empty  
    }
};

// the constructor parses the xml and builds nodes and ways using helper lambdas for parsing attributes  note the double  space
COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) {
    DImplementation = std::make_unique<SImplementation>()  // create the implementation  
    ;  

    std::shared_ptr<SImplementation::SNodeImpl> currentNode = nullptr   // current node pointer  
    ;  
    std::shared_ptr<SImplementation::SWayImpl> currentWay = nullptr   // current way pointer  
    ;  

    // lambda to parse node attributes from xml attributes  
    auto parse_node_attrs = [&] (const std::vector<std::pair<std::string, std::string>> &attrs, std::shared_ptr<SImplementation::SNodeImpl> node) {
         for (auto &attr : attrs) {
             if (attr.first == "id") {
                  node->idVal = std::stoull(attr.second)  // set node id  
             } else if (attr.first == "lat") {
                  node->locVal.first = std::stod(attr.second)  // set latitude  
             } else if (attr.first == "lon") {
                  node->locVal.second = std::stod(attr.second)  // set longitude  
             } else {
                  node->attrib[attr.first] = attr.second  // add extra attribute  
             }
         }
    }  // end of lambda  

    // lambda to parse way attributes from xml attributes  
    auto parse_way_attrs = [&] (const std::vector<std::pair<std::string, std::string>> &attrs, std::shared_ptr<SImplementation::SWayImpl> way) {
         for (auto &attr : attrs) {
             if (attr.first == "id") {
                  way->idVal = std::stoull(attr.second)  // set way id  
             } else {
                  way->attrib[attr.first] = attr.second  // add extra attribute  
             }
         }
    }  // end of lambda  

    SXMLEntity entity   // entity from xml  
    ;  

    while (src->ReadEntity(entity)) {
         if (entity.DType == SXMLEntity::EType::StartElement) {
              if (entity.DNameData == "node") {
                    currentNode = std::make_shared<SImplementation::SNodeImpl>()  // create a new node  
                    ;  
                    currentWay.reset()  // reset way pointer  
                    ;  
                    parse_node_attrs(entity.DAttributes, currentNode)  // parse node attributes  
              } else if (entity.DNameData == "way") {
                    currentWay = std::make_shared<SImplementation::SWayImpl>()  // create a new way  
                    ;  
                    currentNode.reset()  // reset node pointer  
                    ;  
                    parse_way_attrs(entity.DAttributes, currentWay)  // parse way attributes  
              } else if (entity.DNameData == "nd" && currentWay) {
                    for (auto &attr : entity.DAttributes) {
                        if (attr.first == "ref") {
                            currentWay->nodes.push_back(std::stoull(attr.second))  // add node reference  
                        }
                    }
              } else if (entity.DNameData == "tag") {
                    std::string key   // key for tag  
                    , value   // value for tag  
                    ;  
                    for (auto &attr : entity.DAttributes) {
                         if (attr.first == "k") {
                             key = attr.second  // set key  
                         } else if (attr.first == "v") {
                             value = attr.second  // set value  
                         }
                    }
                    if (!key.empty()) {
                        if (currentNode) {
                             currentNode->attrib[key] = value  // add tag to node  
                        } else if (currentWay) {
                             currentWay->attrib[key] = value  // add tag to way  
                        }
                    }
              }
         } else if (entity.DType == SXMLEntity::EType::EndElement) {
              if (entity.DNameData == "node" && currentNode) {
                   DImplementation->nodeList.push_back(currentNode)  // store the node  
                   currentNode.reset()  // reset node pointer  
              } else if (entity.DNameData == "way" && currentWay) {
                   DImplementation->wayList.push_back(currentWay)  // store the way  
                   currentWay.reset()  // reset way pointer  
              }
         }
    }
}

COpenStreetMap::~COpenStreetMap() = default  // default destructor  

// returns the number of nodes in our map
std::size_t COpenStreetMap::NodeCount() const noexcept {
    return DImplementation->nodeList.size()  // return node count  
}

// returns the number of ways in our map
std::size_t COpenStreetMap::WayCount() const noexcept {
    return DImplementation->wayList.size()  // return way count  
}

// returns a node by its index or null if index is out of range
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept {
    return index < DImplementation->nodeList.size() ? DImplementation->nodeList[index] : nullptr  
}

// returns a node with the matching id or null if none found
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept {
    auto it = std::find_if(DImplementation->nodeList.begin(), DImplementation->nodeList.end(), [id](const std::shared_ptr<SImplementation::SNodeImpl> &node) {
         return node->ID() == id  // check if node id matches  
    })  // find the node  
    return it != DImplementation->nodeList.end() ? *it : nullptr  
}

// returns a way by its index or null if index is out of range
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept {
    return index < DImplementation->wayList.size() ? DImplementation->wayList[index] : nullptr  
}

// returns a way with the matching id or null if none found
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept {
    auto it = std::find_if(DImplementation->wayList.begin(), DImplementation->wayList.end(), [id](const std::shared_ptr<SImplementation::SWayImpl> &way) {
         return way->ID() == id  // check if way id matches  
    })  // find the way  
    return it != DImplementation->wayList.end() ? *it : nullptr  
}
