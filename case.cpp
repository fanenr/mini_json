#include <iostream>
#include <string>
#include "json_parser.hpp"


int main()
{
    tiny_json json("{\"skills\":[\"C++\",[\"hello\",\"world\"],{\"food\":\"rice\",\"vegetables\":\"tomato\"},\"C\"],\"age\":19,\"name\":\"wufan\"}");
    json.parse();
    auto str = json.str();
    std::cout << str << std::endl;
}