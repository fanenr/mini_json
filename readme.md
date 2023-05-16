# mini_json is a header-only json library
1. Prerequisites
```
1. CMake 3.15 or later
2. vcpkg (need Catch2 for test)
3. g++ and make (C++17 or later)
4. valgrind (for memory check)
5. use boost::optional replace std::optional
```
1. Build
``` shell
mkdir build && cd build
cmake ..
```
1. Run test
``` shell
# cd build

# run test
make run_t

# run benchmark
make run_b

# run memory check
make run_m
```
4. Demo
``` C++
#include "include/mini_json/json.hpp"
#include <iostream>
#include <cassert>

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
            auto name = *root.at("name").get_str();
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
        auto name = *root["name"].get_str();
        int  age  = *root["age"].get_num();
        std::cout << "name: " << name << std::endl;
        std::cout << "age : " << age << std::endl;
    }
}
```