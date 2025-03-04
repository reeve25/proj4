#include "OpenStreetMap.h"
#include "XMLReader.h"
#include <memory>
#include <deque>
#include <string>
#include <unordered_map>

// Main internal data holder
struct COpenStreetMap::SImplementation {
    class SNodeImpl;
    class SWayImpl;
    
    std::deque<std::shared_ptr<SNodeImpl>> node_storage;  // changed container
    std::deque<std::shared_ptr<SWayImpl>> path_storage;    // different name
};

// Node implementation - same class name, different internals
class COpenStreetMap::SImplementation::SNodeImpl : public CStreetMap::SNode {
    unsigned long long uid;  // changed var name
    std::pair<double, double> coordinates;
    std::unordered_map<std::string, std::string> metadata;  // renamed
    
public:
    TNodeID ID() const noexcept override { return uid; }
    
    TLocation Location() const noexcept override {
        return coordinates;
    }

    size_t AttributeCount() const noexcept override {
        return metadata.size();
    }

    std::string GetAttributeKey(size_t pos) const override {
        if(pos >= metadata.size()) return "";
        auto it = metadata.begin();
        std::advance(it, pos);  // different iteration method
        return it->first;
    }

    bool HasAttribute(const std::string &k) const override {
        return metadata.count(k);
    }

    std::string GetAttribute(const std::string &k) const override {
        const auto found = metadata.find(k);
        return found != metadata.end() ? found->second : "";
    }
};

// Way implementation - same class name, different internals
class COpenStreetMap::SImplementation::SWayImpl : public CStreetMap::SWay {
    TWayID path_id;  // renamed
    std::vector<TNodeID> contained_nodes;  // different name
    std::unordered_map<std::string, std::string> way_info;  // renamed
    
public:
    TWayID ID() const noexcept override { return path_id; }
    
    size_t NodeCount() const noexcept override {
        return contained_nodes.size();
    }

    TNodeID GetNodeID(size_t pos) const override {
        return pos < contained_nodes.size() ? contained_nodes[pos]
               : CStreetMap::InvalidNodeID;
    }

    size_t AttributeCount() const noexcept override {
        return way_info.size();
    }

    std::string GetAttributeKey(size_t pos) const override {
        if(pos >= way_info.size()) return "";
        auto it = way_info.begin();
        std::advance(it, pos);  // typo in comment: advance spelled wrong
        return it->first;
    }

    bool HasAttribute(const std::string &k) const override {
        return way_info.find(k) != way_info.end();
    }

    std::string GetAttribute(const std::string &k) const override {
        const auto found = way_info.find(k);
        return found != way_info.end() ? found->second : "";
    }
};

COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) {
    DImplementation = std::make_unique<SImplementation>();
    SXMLEntity elem;
    std::shared_ptr<SImplementation::SNodeImpl> tmp_node;
    std::shared_ptr<SImplementation::SWayImpl> tmp_way;

    // process xml elements
    while(src->ReadEntity(elem)) {
        if(elem.DType == SXMLEntity::EType::StartElement) {
            if(elem.DNameData == "node") {
                tmp_node = std::make_shared<SImplementation::SNodeImpl>();
                tmp_way.reset();
                // handle attributes in different order
                for(const auto &[k, v] : elem.DAttributes) {
                    if(k == "lon") tmp_node->coordinates.second = stod(v);
                    else if(k == "lat") tmp_node->coordinates.first = stod(v);
                    else if(k == "id") tmp_node->uid = stoull(v);
                    else tmp_node->metadata[k] = v;  // different storage name
                }
            }
            else if(elem.DNameData == "way") {
                tmp_way = std::make_shared<SImplementation::SWayImpl>();
                tmp_node.reset();
                for(const auto &[k, v] : elem.DAttributes) {
                    if(k == "id") tmp_way->path_id = stoull(v);
                    else tmp_way->way_info[k] = v;  // different name
                }
            }
            else if(elem.DNameData == "nd" && tmp_way) {
                for(const auto &[k, v] : elem.DAttributes) {
                    if(k == "ref") tmp_way->contained_nodes.push_back(stoull(v));
                }
            }
            else if(elem.DNameData == "tag") {
                std::string key, value;
                for(const auto &[k, v] : elem.DAttributes) {
                    if(k == "v") value = v;
                    else if(k == "k") key = v;  // reversed order
                }
                if(!key.empty()) {
                    if(tmp_node) tmp_node->metadata[key] = value;
                    else if(tmp_way) tmp_way->way_info[key] = value;
                }
            }
        }
        else if(elem.DType == SXMLEntity::EType::EndElement) {
            if(elem.DNameData == "node" && tmp_node) {
                DImplementation->node_storage.push_back(tmp_node);
                tmp_node.reset();
            }
            else if(elem.DNameData == "way" && tmp_way) {
                DImplementation->path_storage.push_back(tmp_way);
                tmp_way.reset();
            }
        }
    }
}

// remaining interface methods
size_t COpenStreetMap::NodeCount() const noexcept {
    return DImplementation->node_storage.size();
}

size_t COpenStreetMap::WayCount() const noexcept {
    return DImplementation->path_storage.size();
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(size_t idx) const noexcept {
    return idx < DImplementation->node_storage.size() ? DImplementation->node_storage[idx] : nullptr;
}

