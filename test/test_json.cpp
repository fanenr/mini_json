#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <mini_json/json.hpp>

namespace json = mini_json;

TEST_CASE("test json initialize", "[json]")
{
    std::ifstream fs("../test/demo/test2.json");
    if (!fs.is_open())
        throw std::runtime_error("can't open file");

    std::string con { std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>() };

    json::json json_obj(std::move(con));
    auto pret = json_obj.parse();
    auto const& node = *pret;

    auto sret = json_obj.str();
    auto const& str = *sret;
    std::ofstream ofs("../test/demo/output.json");
    ofs << str;
}