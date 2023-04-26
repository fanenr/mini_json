#include <catch2/catch_test_macros.hpp>
#include "json_parser.hpp"

json_node node;

TEST_CASE("parse empty json", "[json_parser]")
{
    REQUIRE(json_parse(node, "  ") == parse_code::expect_value_);
}


TEST_CASE("parse null value", "[json_parser]")
{
    REQUIRE(json_parse(node, " \n null  ") == parse_code::ok_);
    REQUIRE(get_type(node) == json_type::null_);

}


TEST_CASE("parse true value", "[json_parser]")
{
    REQUIRE(json_parse(node, " \n\t true ") == parse_code::ok_);
    REQUIRE(get_type(node) == json_type::true_);
}


TEST_CASE("parse false value", "[json_parser]")
{
    REQUIRE(json_parse(node, " \rfalse") == parse_code::ok_);
    REQUIRE(get_type(node) == json_type::false_);
}

TEST_CASE("parse number value", "[json_parser]")
{
    REQUIRE(json_parse(node, "0") == parse_code::ok_);
    REQUIRE(get_number(node) == 0.0);
    REQUIRE(get_type(node) == json_type::number_);

    REQUIRE(json_parse(node, "-0") == parse_code::ok_);
    REQUIRE(get_number(node) == 0.0);
    REQUIRE(get_type(node) == json_type::number_);

    REQUIRE(json_parse(node, "-0.0") == parse_code::ok_);
    REQUIRE(get_number(node) == 0.0);
    REQUIRE(get_type(node) == json_type::number_);

    REQUIRE(json_parse(node, "112") == parse_code::ok_);
    REQUIRE(get_number(node) == 112.0);
    REQUIRE(get_type(node) == json_type::number_);

    REQUIRE(json_parse(node, "-234") == parse_code::ok_);
    REQUIRE(get_number(node) == -234.0);
    REQUIRE(get_type(node) == json_type::number_);

    REQUIRE(json_parse(node, "123.01") == parse_code::ok_);
    REQUIRE(get_number(node) == 123.01);
    REQUIRE(get_type(node) == json_type::number_);

    REQUIRE(json_parse(node, "-923.89") == parse_code::ok_);
    REQUIRE(get_number(node) == -923.89);
    REQUIRE(get_type(node) == json_type::number_);
}


TEST_CASE("parse string value", "[json_parser]")
{
    REQUIRE(json_parse(node, "\"\"") == parse_code::ok_);
    REQUIRE(get_string(node) == "");
    REQUIRE(get_type(node) == json_type::string_);


    REQUIRE(json_parse(node, "\"Hello\"") == parse_code::ok_);
    REQUIRE(get_string(node) == "Hello");
    REQUIRE(get_type(node) == json_type::string_);

    REQUIRE(json_parse(node, "\"Hello\\nWorld\"") == parse_code::ok_);
    REQUIRE(get_string(node) == "Hello\nWorld");
    REQUIRE(get_type(node) == json_type::string_);

    REQUIRE(json_parse(node, "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"") == parse_code::ok_);
    REQUIRE(get_string(node) == "\" \\ / \b \f \n \r \t");
    REQUIRE(get_type(node) == json_type::string_);
}