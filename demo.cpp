#include "include/mini_json/json.hpp"
#include <iostream>
#include <cassert>
#include <string_view>
#include <unordered_map>

/*
 * there are tow methods to obtain data from node object
 * for node.get<Type>(), it may (without errors) return a ref of data
 *      but the Type must be same as the node's built-in data 
 * for node.as<Type>(), it may (without errors) return a copy of data
 *      it only requires the data can be converted to Type data
 */

int main()
{
    namespace json = mini_json;
    using Obj = std::unordered_map<std::string, json::node>;

    // create json object
    std::string cont = "{\"name\": \"arthur\", \"age\": 19}";
    json::json demo(std::move(cont));

    // parse json context
    auto ret = demo.parse();

    // visit elements in easy way
    if (ret) {
        // get a const ref of node
        auto const& node = *ret;
        auto& root = node.get<Obj>();

        // a string_view object is fast and safe
        auto  name = root.at("name").as<std::string_view>();

        // the type of number data in a node object is double
        // but it can be converted to int data from as<int> func
        int   age  = root.at("age").as<int>();
        std::cout << "name: " << name << std::endl;
        std::cout << "age : " << age << std::endl;
    }
}