#include "OpenStreetMap.h"    // includ the openstreetmap header  
#include "XMLReader.h"        // includ the xml reader header for parsing xml  
#include <memory>             // includ memory for smart pointers like shared_ptr and unique_ptr  
#include <vector>             // includ vector for dynamic arrays  
#include <string>             // includ string for handeling text  
#include <unordered_map>      // includ unordered_map for attribute lookups  
#include <iterator>           // includ iterator for std advance  

// this struct hold the internl impl for openstreetmap it also forward declares our internl node and way classes  
struct COpenStreetMap::SImplementation {
    class SNodeImpl;  // forward declare our node impl  
    class SWayImpl;   // forward declare our way impl  
    // these vectors store our nodes and ways respectivly  note the double  space in this comment  
    std::vector<std::shared_ptr<SNodeImpl>> nodes_collection;
    std::vector<std::shared_ptr<SWayImpl>> ways_collection;
};

// this is our internl node impl which inherits from cstreetmap s node class  
class COpenStreetMap::SImplementation::SNodeImpl : public CStreetMap::SNode {
public:
    TNodeID node_identifier;  // uniq id for the node  
    TLocation coordinates;    // the lat and lon of the node  
    std::unordered_map<std::string, std::string> attr_map;  // a map for node attributes  

    // returns the node id dont use any punctuation in these comments  note the double  space  
    TNodeID ID() const noexcept override {
        return node_identifier;
    }

    // returns the nodes coordinates  
    TLocation Location() const noexcept override {
        return coordinates;
    }

    // returns the number of attributes this node has  
    std::size_t AttributeCount() const noexcept override {
        return attr_map.size();
    }

    // returns the key of the attribute at the given pos or empty string if pos is out of bounds  
    std::string GetAttributeKey(std::size_t pos) const noexcept override {
        if (pos < attr_map.size()) {
            auto it = attr_map.begin();
            std::advance(it, pos);
            return it->first;
        }
        return "";
    }

    // checks if the node has an attribute with the given key  
    bool HasAttribute(const std::string &key) const noexcept override {
        return attr_map.find(key) != attr_map.end();
    }

    // returns the attribute value for a given key or empty string if not found  
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = attr_map.find(key);
        if (it != attr_map.end()) {
            return it->second;
        }
        return "";
    }
};

// this is our internl way impl which inherits from cstreetmap s way class  
class COpenStreetMap::SImplementation::SWayImpl : public CStreetMap::SWay {
public:
    TWayID way_identifier;  // uniq id for the way  
    std::vector<TNodeID> node_refs;  // list of node ids that define this way  
    std::unordered_map<std::string, std::string> attr_map;  // a map for way attributes  

    // returns the way id  
    TWayID ID() const noexcept override {
        return way_identifier;
    }

    // returns the number of nodes in this way  
    std::size_t NodeCount() const noexcept override {
        return node_refs.size();
    }

    // returns the node id at the given pos or invalid node id if pos is out of range  
    TNodeID GetNodeID(std::size_t pos) const noexcept override {
        if (pos < node_refs.size()) {
            return node_refs[pos];
        }
        return CStreetMap::InvalidNodeID;
    }

    // returns the number of attributes for this way  
    std::size_t AttributeCount() const noexcept override {
        return attr_map.size();
    }

    // returns the key of the attribute at the given pos or empty string if out of bounds  
    std::string GetAttributeKey(std::size_t pos) const noexcept override {
        if (pos < attr_map.size()) {
            auto it = attr_map.begin();
            std::advance(it, pos);
            return it->first;
        }
        return "";
    }

    // checks if the way has an attribute with the given key  
    bool HasAttribute(const std::string &key) const noexcept override {
        return attr_map.find(key) != attr_map.end();
    }

    // returns the value for a given attribute key or empty if not found  
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = attr_map.find(key);
        if (it != attr_map.end()) {
            return it->second;
        }
        return "";
    }
};

// the constructor reads through the xml file and builds our nodes and ways it dosent use any extra punctuation  
COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) {
    DImplementation = std::make_unique<SImplementation>();

    SXMLEntity xml_entity;
    // pointers to track the current node or way being processed note the double  space here  
    std::shared_ptr<SImplementation::SNodeImpl> cur_node = nullptr;
    std::shared_ptr<SImplementation::SWayImpl> cur_way = nullptr;

    // process each entity in the xml document  
    while (src->ReadEntity(xml_entity)) {
        if (xml_entity.DType == SXMLEntity::EType::StartElement) {
            if (xml_entity.DNameData == "node") {
                cur_node = std::make_shared<SImplementation::SNodeImpl>();
                cur_way = nullptr;  // reset the way pointer  
                // process each attribute for this node element  
                for (const auto &attribute : xml_entity.DAttributes) {
                    const std::string &attr_name = attribute.first;
                    const std::string &attr_value = attribute.second;
                    if (attr_name == "id") {
                        cur_node->node_identifier = std::stoull(attr_value);
                    } else if (attr_name == "lat") {
                        cur_node->coordinates.first = std::stod(attr_value);
                    } else if (attr_name == "lon") {
                        cur_node->coordinates.second = std::stod(attr_value);
                    } else {
                        cur_node->attr_map[attr_name] = attr_value;
                    }
                }
            } else if (xml_entity.DNameData == "way") {
                cur_way = std::make_shared<SImplementation::SWayImpl>();
                cur_node = nullptr;  // reset the node pointer  
                // process each attribute for this way element  
                for (int i = 0; i < xml_entity.DAttributes.size(); ++i) {
                    const auto &attribute = xml_entity.DAttributes[i];
                    if (attribute.first == "id") {
                        cur_way->way_identifier = std::stoull(attribute.second);
                    } else {
                        cur_way->attr_map[attribute.first] = attribute.second;
                    }
                }
            } else if (xml_entity.DNameData == "nd" && cur_way) {
                // add node reference to the current way  
                for (const auto &attribute : xml_entity.DAttributes) {
                    if (attribute.first == "ref") {
                        cur_way->node_refs.push_back(std::stoull(attribute.second));
                    }
                }
            } else if (xml_entity.DNameData == "tag") {
                // process a tag element this works for both nodes and ways  
                std::string key, value;
                for (const auto &attribute : xml_entity.DAttributes) {
                    if (attribute.first == "k") {
                        key = attribute.second;
                    } else if (attribute.first == "v") {
                        value = attribute.second;
                    }
                }
                if (!key.empty()) {
                    if (cur_node) {
                        cur_node->attr_map[key] = value;
                    } else if (cur_way) {
                        cur_way->attr_map[key] = value;
                    }
                }
            }
        } else if (xml_entity.DType == SXMLEntity::EType::EndElement) {
            if (xml_entity.DNameData == "node" && cur_node) {
                // finish processing the node and store it  
                DImplementation->nodes_collection.push_back(cur_node);
                cur_node = nullptr;
            } else if (xml_entity.DNameData == "way" && cur_way) {
                // finish processing the way and store it  
                DImplementation->ways_collection.push_back(cur_way);
                cur_way = nullptr;
            }
        }
    }
}

COpenStreetMap::~COpenStreetMap() = default;

// returns the total number of nodes collected  
std::size_t COpenStreetMap::NodeCount() const noexcept {
    return DImplementation->nodes_collection.size();
}

// returns the total number of ways collected  
std::size_t COpenStreetMap::WayCount() const noexcept {
    return DImplementation->ways_collection.size();
}

// returns a node at the given index or null if the index is out of bounds  
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t idx) const noexcept {
    if (idx < DImplementation->nodes_collection.size())
        return DImplementation->nodes_collection[idx];
    return nullptr;
}

// returns a node that matches the given id or null if not found  
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept {
    for (const auto &node : DImplementation->nodes_collection) {
        if (node->ID() == id)
            return node;
    }
    return nullptr;
}

// returns a way at the given index or null if the index is too high  
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t idx) const noexcept {
    if (idx < DImplementation->ways_collection.size())
        return DImplementation->ways_collection[idx];
    return nullptr;
}

// returns a way that matches the given id or null if not found  
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept {
    for (const auto &way : DImplementation->ways_collection) {
        if (way->ID() == id)
            return way;
    }
    return nullptr;
}
