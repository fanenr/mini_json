#include <iostream>
#include <string>
#include "json_parser.hpp"

int main()
{
    json_node node;
    json_parse(node, "{\"name\": \"wufan\", \"age\": 19, \"skills\": [\"C++\", \"C\"]}");
    
    

    for (auto& v : node.data.member) {
        std::cout << "key: " << v.first << std::endl;
        std::cout << "val: ";
        if (v.second.type == json_type::string_)
            std::cout << v.second.data.string << std::endl;
        if (v.second.type == json_type::number_)
            std::cout << v.second.data.number << std::endl;
        if (v.second.type == json_type::array_)
            for (auto& j : v.second.data.array) {
                std::cout << std::endl << "     " << j.data.string << std::endl;
            }
    }
}