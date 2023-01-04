#include <gtest/gtest.h>

#include "Config.h"
#include "Individual.h"
#include "ProblemData.h"

TEST(IndividualTest, routeConstructorSortsByEmpty)
{
    auto const data = ProblemData::fromFile(Config{}, "data/OkSmall.txt");
    std::vector<std::vector<int>> const routes = {
        {3, 4},
        {},
        {1, 2},
    };

    Individual indiv{data, routes};
    auto const &indivRoutes = indiv.getRoutes();

    // numRoutes() should show two non-empty routes. We passed-in three routes,
    // however, so indivRoutes.size() should not have changed.
    ASSERT_EQ(indiv.numRoutes(), 2);
    ASSERT_EQ(indivRoutes.size(), 3);

    // We expect Individual to sort the routes such that all non-empty routes
    // are in the lower indices.
    EXPECT_EQ(indivRoutes[0].size(), 2);
    EXPECT_EQ(indivRoutes[1].size(), 2);
    EXPECT_EQ(indivRoutes[2].size(), 0);
}

TEST(IndividualTest, routeConstructorThrows)
{
    auto const data = ProblemData::fromFile(Config{}, "data/OkSmall.txt");
    ASSERT_EQ(data.nbVehicles, 3);

    // Two routes, three vehicles: should throw.
    ASSERT_THROW((Individual{data, {{1, 2}, {4, 2}}}), std::runtime_error);

    // Empty third route: should not throw.
    ASSERT_NO_THROW((Individual{data, {{1, 2}, {4, 2}, {}}}));
}

TEST(IndividualTest, getNeighbours)
{
    auto const data = ProblemData::fromFile(Config{}, "data/OkSmall.txt");
    std::vector<std::vector<int>> const routes = {
        {3, 4},
        {},
        {1, 2},
    };

    Individual indiv{data, routes};
    auto const &neighbours = indiv.getNeighbours();
    std::vector<std::pair<int, int>> expected = {
        {0, 0},  // 0: is depot
        {0, 2},  // 1: between depot (0) to 2
        {1, 0},  // 2: between 1 and depot (0)
        {0, 4},  // 3: between depot (0) and 4
        {3, 0},  // 4: between 3 and depot (0)
    };

    for (auto client = 0; client != 5; ++client)
        EXPECT_EQ(neighbours[client], expected[client]);
}

TEST(IndividualTest, feasibility)
{
    auto const data = ProblemData::fromFile(Config{}, "data/OkSmall.txt");

    // This solution is infeasible due to both load and time window violations.
    std::vector<std::vector<int>> const routes = {{1, 2, 3, 4}, {}, {}};
    Individual indiv{data, routes};
    EXPECT_FALSE(indiv.isFeasible());

    // First route has total load 18, but vehicle capacity is only 10.
    EXPECT_TRUE(indiv.hasExcessCapacity());

    // Client 4 has TW [8400, 15300], but client 2 cannot be visited before
    // 15600, so there must be time warp on the single-route solution.
    EXPECT_TRUE(indiv.hasTimeWarp());

    // Let's try another solution that's actually feasible.
    std::vector<std::vector<int>> const routes2 = {{1, 2}, {3}, {4}};
    Individual indiv2{data, routes2};
    EXPECT_TRUE(indiv2.isFeasible());
    EXPECT_FALSE(indiv2.hasExcessCapacity());
    EXPECT_FALSE(indiv2.hasTimeWarp());
}

TEST(IndividualTest, brokenPairsDistance)
{
    auto const data = ProblemData::fromFile(Config{}, "data/OkSmall.txt");

    std::vector<std::vector<int>> const routes1 = {{1, 2, 3, 4}, {}, {}};
    Individual indiv1{data, routes1};

    std::vector<std::vector<int>> const routes2 = {{1, 2}, {3}, {4}};
    Individual indiv2{data, routes2};

    // Compare indiv1 and indiv2. The two broken pairs are (2, 3) and (3, 4).
    EXPECT_EQ(indiv1.brokenPairsDistance(&indiv2), 2);
    EXPECT_EQ(indiv2.brokenPairsDistance(&indiv1), 2);  // should be symmetric

    std::vector<std::vector<int>> const routes3 = {{3}, {4, 1, 2}, {}};
    Individual indiv3{data, routes3};

    // Compare indiv1 and indiv3. The three broken pairs are (0, 1), (2, 3),
    // and (3, 4).
    EXPECT_EQ(indiv1.brokenPairsDistance(&indiv3), 3);
    EXPECT_EQ(indiv3.brokenPairsDistance(&indiv1), 3);  // should be symmetric

    // Compare indiv2 and indiv3. The broken pair is (0, 1).
    EXPECT_EQ(indiv2.brokenPairsDistance(&indiv3), 1);
    EXPECT_EQ(indiv3.brokenPairsDistance(&indiv2), 1);  // should be symmetric
}

TEST(IndividualCostTest, distance)
{
    auto const data = ProblemData::fromFile(Config{}, "data/OkSmall.txt");
    std::vector<std::vector<int>> const routes = {{1, 2}, {3}, {4}};
    Individual indiv{data, routes};

    ASSERT_TRUE(indiv.isFeasible());

    // This individual is feasible, so cost should equal total distance
    // travelled.
    int dist = data.dist(0, 1, 2, 0) + data.dist(0, 3, 0) + data.dist(0, 4, 0);
    EXPECT_EQ(dist, indiv.cost());
}

TEST(IndividualCostTest, capacity)
{
    Config const config;
    auto const data = ProblemData::fromFile(config, "data/OkSmall.txt");
    std::vector<std::vector<int>> const routes = {{4, 3, 1, 2}, {}, {}};
    Individual indiv{data, routes};

    ASSERT_TRUE(indiv.hasExcessCapacity());
    ASSERT_FALSE(indiv.hasTimeWarp());

    int load = 0;

    for (auto &client : data.clients)  // all demand, since all clients are
        load += client.demand;         // in a single route

    int excessLoad = load - data.vehicleCapacity;
    int loadPenalty = config.initialCapacityPenalty * excessLoad;
    int dist = data.dist(0, 4, 3, 1, 2, 0);

    // This individual is infeasible due to load violations, so the costs should
    // be distance + loadPenalty * excessLoad.
    EXPECT_EQ(dist + loadPenalty, indiv.cost());
}

TEST(IndividualCostTest, timeWarp)
{
    Config const config;
    auto const data = ProblemData::fromFile(config, "data/OkSmall.txt");
    std::vector<std::vector<int>> const routes = {{1, 3}, {2, 4}, {}};
    Individual indiv{data, routes};

    ASSERT_FALSE(indiv.hasExcessCapacity());
    ASSERT_TRUE(indiv.hasTimeWarp());

    // There's only time warp on the first route: dist(0, 1) = 1'544, so we
    // arrive at 1 before its opening window of 15'600. Service (360) thus
    // starts at 15'600, and completes at 15'600 + 360. Then we drive
    // dist(1, 3) = 1'427, where we arrive after 15'300 (its closing time
    // window). This is where we incur time warp: we need to 'warp back' to
    // 15'300.
    int twR1 = 15'600 + 360 + 1'427 - 15'300;
    int twR2 = 0;
    int timeWarp = twR1 + twR2;
    int twPenalty = config.initialTimeWarpPenalty * timeWarp;
    int dist = data.dist(0, 1, 3, 0) + data.dist(0, 2, 4, 0);

    // This individual is infeasible due to time warp, so the costs should
    // be distance + twPenalty * timeWarp.
    EXPECT_EQ(dist + twPenalty, indiv.cost());

    // TODO test all time warp cases
}