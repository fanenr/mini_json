#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <memory>
#include <mini_json/node.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace json = mini_json;

TEST_CASE("test node null", "[node]")
{
    json::node null1;
    json::node null2(nullptr);
    json::node null3(std::nullptr_t {});

    REQUIRE(null1.get<std::nullptr_t>() == nullptr);
    REQUIRE(null2.as<std::nullptr_t>() == nullptr);
    REQUIRE(null3.as<std::nullptr_t>() == nullptr);
}

TEST_CASE("test node bool", "[node]")
{
    json::node bool1(true);
    json::node bool2(false);

    REQUIRE(bool1.as<bool>() == true);
    REQUIRE(bool1.get<bool>() == true);

    REQUIRE(bool2.get<bool>() == false);
    REQUIRE(bool2.as<bool>() == false);
}

TEST_CASE("test node number", "[node]")
{
    json::node num1(11);
    json::node num2(2.0f);
    json::node num3(1.0);

    REQUIRE(num1.as<int>() == 11);
    REQUIRE(num2.as<float>() == 2.0f);
    REQUIRE(num3.get<double>() == 1.0);
}

TEST_CASE("test node string", "[node]")
{
    std::string world = "world";
    json::node str1("hello");
    json::node str2(world);
    json::node str3(std::move(world));

    REQUIRE(str1.get<std::string>() == "hello");
    REQUIRE(str2.as<std::string>() == "world");
    REQUIRE(str3.as<std::string_view>() == "world");
}

TEST_CASE("test node array", "[node]")
{
    std::vector<json::node> vec { {}, 1, "hello" };
    json::node arr1(vec);
    json::node arr2(std::move(vec));

    {
        auto& node = arr1.get<std::vector<json::node>>();
        REQUIRE(node[2].get<std::string>() == "hello");
    }
}

TEST_CASE("test node object", "[node]")
{
    std::unordered_map<std::string, json::node> map = {
        { "name", "arthur" },
        { "age", 19 }
    };

    json::node obj1(map);
    json::node obj2(std::move(map));

    {
        auto& node = obj1.get<std::unordered_map<std::string, json::node>>();
        REQUIRE(node["name"].get<std::string>() == "arthur");
        REQUIRE(node["age"].as<int>() == 19);
    }
}