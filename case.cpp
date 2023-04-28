#include <iostream>
#include <fstream>
#include <string>
#include "json_parser.hpp"


int main()
{
    std::ifstream file("test.txt");
    std::string test_json;
    std::getline(file, test_json);

    tiny_json json(test_json);
    json.parse();
    auto str = json.str();

    tiny_json json2(std::move(str));
    json2.parse();
    auto str2 = json2.str();
    
    std::cout << str2 << std::endl;
}