#include "gtest/gtest.h"
#include "DijkstraPathRouter.h"
#include <chrono>
#include <string>
#include <vector>

class DijkstraPathRouterTest : public ::testing::Test {
protected:
    CDijkstraPathRouter router;
    
    void SetUp() override {
        // Setup is done in individual tests as needed
    }
};

// Test vertex creation and counting
TEST_F(DijkstraPathRouterTest, VertexCreation) {
    EXPECT_EQ(0, router.VertexCount());
    
    auto v1 = router.AddVertex("Vertex 1");
    EXPECT_EQ(1, router.VertexCount());
    
    auto v2 = router.AddVertex("Vertex 2");
    auto v3 = router.AddVertex("Vertex 3");
    EXPECT_EQ(3, router.VertexCount());
    
    // Test vertex IDs are assigned correctly
    EXPECT_EQ(0, v1);
    EXPECT_EQ(1, v2);
    EXPECT_EQ(2, v3);
}

// Test vertex tag retrieval
TEST_F(DijkstraPathRouterTest, VertexTags) {
    auto v1 = router.AddVertex("Vertex 1");
    auto v2 = router.AddVertex(42);
    auto v3 = router.AddVertex(3.14);
    
    // Test tag retrieval using std::any
    EXPECT_EQ("Vertex 1", std::any_cast<std::string>(router.GetVertexTag(v1)));
    EXPECT_EQ(42, std::any_cast<int>(router.GetVertexTag(v2)));
    EXPECT_EQ(3.14, std::any_cast<double>(router.GetVertexTag(v3)));
}

// Test edge creation
TEST_F(DijkstraPathRouterTest, EdgeCreation) {
    auto v1 = router.AddVertex("A");
    auto v2 = router.AddVertex("B");
    
    // Test adding a valid edge
    EXPECT_TRUE(router.AddEdge(v1, v2, 10.0));
    
    // Test adding an invalid edge (weight <= 0)
    EXPECT_FALSE(router.AddEdge(v1, v2, 0.0));
    EXPECT_FALSE(router.AddEdge(v1, v2, -5.0));
    
    // Test bidirectional edge
    auto v3 = router.AddVertex("C");
    EXPECT_TRUE(router.AddEdge(v2, v3, 5.0, true));
}

// Test shortest path in a simple graph
TEST_F(DijkstraPathRouterTest, SimplePathFinding) {
    // Create a simple path: A -> B -> C
    auto vA = router.AddVertex("A");
    auto vB = router.AddVertex("B");
    auto vC = router.AddVertex("C");
    
    router.AddEdge(vA, vB, 10.0);
    router.AddEdge(vB, vC, 15.0);
    
    std::vector<CPathRouter::TVertexID> path;
    double distance = router.FindShortestPath(vA, vC, path);
    
    EXPECT_EQ(25.0, distance);
    EXPECT_EQ(3, path.size());
    EXPECT_EQ(vA, path[0]);
    EXPECT_EQ(vB, path[1]);
    EXPECT_EQ(vC, path[2]);
}

// Test shortest path with alternatives
TEST_F(DijkstraPathRouterTest, AlternativePathFinding) {
    // Create a diamond graph: A -> B -> D
    //                         \-> C ->/
    auto vA = router.AddVertex("A");
    auto vB = router.AddVertex("B");
    auto vC = router.AddVertex("C");
    auto vD = router.AddVertex("D");
    
    router.AddEdge(vA, vB, 10.0);
    router.AddEdge(vA, vC, 5.0);
    router.AddEdge(vB, vD, 10.0);
    router.AddEdge(vC, vD, 15.0);
    
    std::vector<CPathRouter::TVertexID> path;
    double distance = router.FindShortestPath(vA, vD, path);
    
    // The shortest path should be A -> B -> D (total: 20.0)
    // Not A -> C -> D (total: 20.0) - if equal distances, implementation details determine which is chosen
    EXPECT_EQ(20.0, distance);
    EXPECT_EQ(3, path.size());
}

// Test non-existent path
TEST_F(DijkstraPathRouterTest, NoPathExists) {
    auto vA = router.AddVertex("A");
    auto vB = router.AddVertex("B");
    
    // No edge between A and B
    
    std::vector<CPathRouter::TVertexID> path;
    double distance = router.FindShortestPath(vA, vB, path);
    
    EXPECT_EQ(CPathRouter::NoPathExists, distance);
    EXPECT_TRUE(path.empty());
}

// Test path to self
TEST_F(DijkstraPathRouterTest, PathToSelf) {
    auto vA = router.AddVertex("A");
    
    std::vector<CPathRouter::TVertexID> path;
    double distance = router.FindShortestPath(vA, vA, path);
    
    EXPECT_EQ(0.0, distance);
    EXPECT_EQ(1, path.size());
    EXPECT_EQ(vA, path[0]);
}

// Test bidirectional edges
TEST_F(DijkstraPathRouterTest, BidirectionalEdges) {
    auto vA = router.AddVertex("A");
    auto vB = router.AddVertex("B");
    
    router.AddEdge(vA, vB, 10.0, true);
    
    // Test path A -> B
    std::vector<CPathRouter::TVertexID> path1;
    double distance1 = router.FindShortestPath(vA, vB, path1);
    EXPECT_EQ(10.0, distance1);
    
    // Test path B -> A (should exist due to bidirectional edge)
    std::vector<CPathRouter::TVertexID> path2;
    double distance2 = router.FindShortestPath(vB, vA, path2);
    EXPECT_EQ(10.0, distance2);
}

// Test complex graph
TEST_F(DijkstraPathRouterTest, ComplexGraph) {
    // Create vertices
    auto v0 = router.AddVertex("0");
    auto v1 = router.AddVertex("1");
    auto v2 = router.AddVertex("2");
    auto v3 = router.AddVertex("3");
    auto v4 = router.AddVertex("4");
    auto v5 = router.AddVertex("5");
    
    // Create edges to form a complex graph
    router.AddEdge(v0, v1, 7.0);
    router.AddEdge(v0, v2, 9.0);
    router.AddEdge(v0, v5, 14.0);
    router.AddEdge(v1, v2, 10.0);
    router.AddEdge(v1, v3, 15.0);
    router.AddEdge(v2, v3, 11.0);
    router.AddEdge(v2, v5, 2.0);
    router.AddEdge(v3, v4, 6.0);
    router.AddEdge(v4, v5, 9.0);
    
    std::vector<CPathRouter::TVertexID> path;
    double distance = router.FindShortestPath(v0, v4, path);
    
    // The shortest path should be 0 -> 2 -> 5 -> 4 (total: 20.0)
    EXPECT_EQ(20.0, distance);
    EXPECT_EQ(4, path.size());
    EXPECT_EQ(v0, path[0]);
    EXPECT_EQ(v2, path[1]);
    EXPECT_EQ(v5, path[2]);
    EXPECT_EQ(v4, path[3]);
}

// Test precompute function (mostly a placeholder since the implementation doesn't do much)
TEST_F(DijkstraPathRouterTest, Precomputation) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    EXPECT_TRUE(router.Precompute(deadline));
}

// Test invalid vertex IDs
TEST_F(DijkstraPathRouterTest, InvalidVertexIds) {
    auto v0 = router.AddVertex("0");
    
    std::vector<CPathRouter::TVertexID> path;
    
    // Test with invalid source
    double distance1 = router.FindShortestPath(100, v0, path);
    EXPECT_EQ(CPathRouter::NoPathExists, distance1);
    EXPECT_TRUE(path.empty());
    
    // Test with invalid destination
    double distance2 = router.FindShortestPath(v0, 100, path);
    EXPECT_EQ(CPathRouter::NoPathExists, distance2);
    EXPECT_TRUE(path.empty());
}