#include <boost/optional/optional_io.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <mini_json/node.hpp>

namespace json = mini_json;
using type = json::node_t;

TEST_CASE("test node null", "[node]")
{
    json::node null1;
    REQUIRE(null1.type() == type::null_t);

    json::node null2(json::null_t {});
    REQUIRE(null2.type() == type::null_t);

    json::node null3(nullptr);
    REQUIRE(null3.type() == type::null_t);

    REQUIRE(null1.get<type::null_t>() == json::null_t {});
    REQUIRE(null2.get<type::null_t>() == json::null_t {});
    REQUIRE(null3.get<type::null_t>() == json::null_t {});
}

TEST_CASE("test node bool", "[node]")
{
    json::node bool1(true);
    REQUIRE(bool1.type() == type::bool_t);

    json::node bool2(false);
    REQUIRE(bool2.type() == type::bool_t);

    REQUIRE(bool1.get<type::bool_t>() == true);
    REQUIRE(bool1.get_bool());
    REQUIRE(*bool1.get_bool() == true);

    REQUIRE(bool2.get<type::bool_t>() == false);
    REQUIRE(bool2.get_bool());
    REQUIRE(*bool2.get_bool() == false);
}

TEST_CASE("test node number", "[node]")
{
    json::node num1(11);
    REQUIRE(num1.type() == type::number_t);

    json::node num2(2.0f);
    REQUIRE(num2.type() == type::number_t);

    json::node num3(1.0);
    REQUIRE(num3.type() == type::number_t);

    REQUIRE(num2.get<type::number_t>() == static_cast<json::number_t>(2.0f));
    REQUIRE(num2.get_num());
    REQUIRE(*num2.get_num() == static_cast<json::number_t>(2.0f));
}

TEST_CASE("test node string", "[node]")
{
    json::node str1("hello");
    REQUIRE(str1.type() == type::string_t);

    json::string_t world = "world";
    json::node str2(world);
    REQUIRE(str2.type() == type::string_t);

    json::node str3(std::move(world));
    REQUIRE(str3.type() == type::string_t);

    REQUIRE(str3.get<type::string_t>() == "world");
    REQUIRE(str3.get_str());
    REQUIRE(*str3.get_str() == "world");
}

TEST_CASE("test node array", "[node]")
{
    json::array_t vec { json::node(), json::node(1), json::node("hello") };
    json::node arr1(vec);
    REQUIRE(arr1.type() == type::array_t);

    json::node arr2(std::move(vec));
    REQUIRE(arr2.type() == type::array_t);
}

TEST_CASE("test node object", "[node]")
{
    json::object_t map;
    map.emplace("name", json::node("arthur"));
    map.emplace("age", json::node(19));

    json::node obj1(map);
    REQUIRE(obj1.type() == type::object_t);

    json::node obj2(std::move(map));
    REQUIRE(obj2.type() == type::object_t);
}