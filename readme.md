# mini_json is a header-only json library
1. Prerequisites
```
1. C++17 at least
2. CMake 3.15 or later
3. vcpkg (need Catch2 for test)
4. valgrind (for memory check)
```
2. Build
``` shell
mkdir build && cd build
cmake ..
```
3. Run test
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
        auto& node = *ret;
        auto& root = node.get<Obj>();
        // a string_view object is fast and safe
        auto  name = root["name"].as<std::string_view>();
        // the type of number data in a node object is double
        // but it can be converted to int data from as<int> func
        int   age  = root["age"].as<int>();
        std::cout << "name: " << name << std::endl;
        std::cout << "age : " << age << std::endl;
    }
}
```