#include <catch2/catch_test_macros.hpp>
#include <mini_json/node.hpp>
#include <memory>

namespace json = mini_json;
using     type = json::node_t;


TEST_CASE("test node initialize", "[node]")
{
    // node(nullptr)
    json::node node_null;
    REQUIRE(node_null.type() == type::null_t);
    REQUIRE(node_null.get<type::null_t>() == nullptr);

    // node(null)
    auto null = nullptr;
    json::node node_null2(null);
    REQUIRE(node_null2.type() == type::null_t);
    REQUIRE(node_null2.get<type::null_t>() == nullptr);

    // node(true)
    json::node node_bool(true);
    REQUIRE(node_bool.type() == type::bool_t);
    REQUIRE(node_bool.get<type::bool_t>() == true);

    // ndoe(sts)
    bool sts = true;
    json::node node_bool2(sts);
    REQUIRE(node_bool2.type() == type::bool_t);
    REQUIRE(node_bool2.get<type::bool_t>() == true);

    // node(sts&)
    bool& sts_rl = sts;
    json::node node_bool3(sts_rl);
    REQUIRE(node_bool3.type() == type::bool_t);
    REQUIRE(node_bool3.get<type::bool_t>() == true);

    // node(bool&&)
    bool&& sts_rr = std::move(sts);
    json::node node_bool4(sts_rr);
    REQUIRE(node_bool4.type() == type::bool_t);
    REQUIRE(node_bool4.get<type::bool_t>() == true);

    // node(bool const&)
    bool const& sts_crl = sts;
    json::node node_bool5(sts_crl);
    REQUIRE(node_bool5.type() == type::bool_t);
    REQUIRE(node_bool5.get<type::bool_t>() == true);

    // node(bool const&)
    bool const&& sts_crr = std::move(sts);
    json::node node_bool6(sts_crr);
    REQUIRE(node_bool6.type() == type::bool_t);
    REQUIRE(node_bool6.get<type::bool_t>() == true);

    // ndoe(double)
    double num = 1.0;
    json::node node_num(1.0);
    REQUIRE(node_num.type() == type::number_t);
    REQUIRE(node_num.get<type::number_t>() == 1.0);

    // node(double const&)
    auto const& num_crl = num;
    json::node node_num2(num_crl);
    REQUIRE(node_num2.type() == type::number_t);
    REQUIRE(node_num2.get<type::number_t>() == 1.0);

    // node(double) <- std::unique_ptr<double>
    auto num_ptr = std::make_unique<double>(num_crl);
    json::node node_num3(*num_ptr);
    REQUIRE(node_num3.type() == type::number_t);
    REQUIRE(node_num3.get<type::number_t>() == 1.0);

    // node(string)
    std::string str("hello");
    json::node node_str(str);
    REQUIRE(node_str.type() == type::string_t);
    REQUIRE(node_str.get<type::string_t>() == str);

    // node(string&)
    auto& str_cp = str;
    json::node node_str2(str_cp);
    REQUIRE(node_str2.type() == type::string_t);
    REQUIRE(node_str2.get<type::string_t>() == str_cp);

    // node(string&&)
    json::node node_str3(std::move(str_cp));
    REQUIRE(node_str3.type() == type::string_t);
    REQUIRE(node_str3.get<type::string_t>() == "hello");
    // the str must be empty string because we moved it
    REQUIRE(str == "");

    // node(array)
    std::vector<json::node> vec;
    for (int i = 0; i < 10; ++i)
        vec.emplace_back(json::node(static_cast<double>(i)));
    json::node node_arr(vec);
    REQUIRE(node_arr.type() == type::array_t);
    REQUIRE(node_arr.get<type::array_t>().size() == 10);
    for (int i = 0; i < 10; ++i)
        REQUIRE(static_cast<double>(i) == node_arr.get<type::array_t>()[i].get<type::number_t>());

    // node(array&&)
    json::node node_arr2(std::move(vec));
    REQUIRE(node_arr.type() == type::array_t);
    REQUIRE(node_arr.get<type::array_t>().size() == 10);
    // the size of vec must be 0 because we moved it
    REQUIRE(vec.size() == 0);
    for (int i = 0; i < 10; ++i)
        REQUIRE(static_cast<double>(i) == node_arr2.get<type::array_t>()[i].get<type::number_t>());
}


TEST_CASE("test node get", "[node]")
{

}


TEST_CASE("test node set", "[node]")
{
    json::node node1;
    node1.set(1.0);
    REQUIRE(node1.type() == type::number_t);
    REQUIRE(node1.get<type::number_t>() == 1.0);

    node1.set(true);
    REQUIRE(node1.type() == type::bool_t);
    REQUIRE(node1.get<type::bool_t>() == true);

    node1.set(json::object_t());
}