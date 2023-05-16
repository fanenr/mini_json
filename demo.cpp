#include "include/mini_json/json.hpp"
#include <iostream>
#include <cassert>
#include <string_view>

int main()
{
    namespace json = mini_json;
    using ntype = json::node_t;

    // create json object
    json::context_t cont = "{\"name\": \"arthur\", \"age\": 19}";
    json::json demo(std::move(cont));

    // parse json context
    auto ret = demo.parse();

    // visit elements in safety way
    if (ret) {
        // get node
        auto const& node = *ret;
        // get elements
        try {
            // auto const& root = node.get<ntype::object_t>();
            assert(node.get_obj());
            auto const& root = *node.get_obj();
            // auto name = root.at("name").get<ntype::string_t>();
            // int  age  = root.at("age").get<ntype::number_t>();
            assert(root.at("name").get_str());
            std::string_view name = *root.at("name").get_str();
            int  age  = root.at("age").get_num().value_or(-1);
            std::cout << "name: " << name << std::endl;
            std::cout << "age : " << age << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    }

    // visit elements in easy way
    if (ret) {
        auto& node = *ret;
        auto& root = *node.get_obj();
        auto& name = *root["name"].get_str();
        int   age  = *root["age"].get_num();
        std::cout << "name: " << name << std::endl;
        std::cout << "age : " << age << std::endl;
    }
}