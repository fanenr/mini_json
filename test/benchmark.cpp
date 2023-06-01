#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#include <fstream>
#include <mini_json/json.hpp>

namespace json = mini_json;


TEST_CASE("json test", "[benchmark]")
{
    std::ifstream fs("../test/demo/test2.json");
    if (!fs.is_open())
        throw std::runtime_error("Can't open file");

    std::string con { std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>() };
    json::json obj(std::move(con));

    BENCHMARK("test json parse")
    {
        auto ret = obj.parse();
        return true;
    };
    
    BENCHMARK("test json stringify")
    {
        auto ret = obj.str();
        return true;
    };
}