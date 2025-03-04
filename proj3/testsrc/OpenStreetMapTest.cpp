#include "OpenStreetMap.h"
#include "XMLReader.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>

class MockXMLReader : public CXMLReader {
public:
    std::vector<SXMLEntity> Data;
    size_t CurrentIndex;

    MockXMLReader() : CXMLReader(nullptr), CurrentIndex(0) {}

    // Ensure the method signature matches the base class
    bool ReadEntity(SXMLEntity &entity, bool skipcdata = false) override {
        if (CurrentIndex < Data.size()) {
            entity = Data[CurrentIndex++];
            return true;
        }
        return false;
    }
};

class OpenStreetMapTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(OpenStreetMapTest, TestNodeCount) {
    auto xmlReader = std::make_shared<MockXMLReader>();
    xmlReader->Data = { {SXMLEntity::EType::StartElement, "node"},
                         {SXMLEntity::EType::StartElement, "node"},
                         {SXMLEntity::EType::StartElement, "node"} };

    COpenStreetMap osmMap(xmlReader);
    EXPECT_EQ(osmMap.NodeCount(), 3);
}

TEST_F(OpenStreetMapTest, TestWayCount) {
    auto xmlReader = std::make_shared<MockXMLReader>();
    xmlReader->Data = { {SXMLEntity::EType::StartElement, "way"},
                         {SXMLEntity::EType::StartElement, "way"} };

    COpenStreetMap osmMap(xmlReader);
    EXPECT_EQ(osmMap.WayCount(), 2);
}
