#include <boost/optional/optional_io.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <mini_json/json.hpp>

namespace json = mini_json;
using type = json::node_t;

TEST_CASE("test json initialize", "[json]")
{
    std::ifstream fs("../test/demo/test2.json");
    if (!fs.is_open())
        throw std::runtime_error("Can't open file");

    json::context_t con { std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>() };

    json::json json_obj(std::move(con));
    auto pret = json_obj.parse();
    REQUIRE(pret);
    auto const& node = *pret;

    auto sret = json_obj.str();
    REQUIRE(sret);
    auto const& str = *sret;
    std::ofstream ofs("../test/demo/output.json");
    ofs << str;
}