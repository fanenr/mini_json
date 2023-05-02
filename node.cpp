#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

enum class node_type {
    null_,
    true_,
    false_,
    number_,
    string_,
    array_,
    object_
};

// json node is parsing target
template <typename T>
struct node {
    // the type of node
    node_type type;
    // the type of number
    using number_type = double;
    // the type of string
    using string_type = std::string;
    // the container type of array
    using array_type = std::vector<node>;
    // the type of object key
    using key_type = std::string;
    // the container type of objec
    using object_type = std::unordered_map<key_type, node>;

    // the data of node
    object_type object;
    union {
        number_type number;
        string_type string;
        array_type array;
    };

    // node operation interface
    node()
        : type(node_type::null_)
    {
    }

    

    node(node::number_type num)
        : number(num)
        , type(node_type::number_)
    {
    }

    node(node const& src)
    {
        type = src.type;
        switch (type) {
        case node_type::number_:
            number = src.number;
            break;
        case node_type::string_:
            new (&string) string_type(src.string);
            break;
        case node_type::array_:
            new (&array) array_type(src.array);
            break;
        case node_type::object_:
            new (&object) object_type(src.object);
            break;
        }
    }

    node(node&& src)
    {
        type = src.type;
        switch (type) {
        case node_type::number_:
            number = src.number;
            break;
        case node_type::string_:
            new (&string) string_type(std::move(src.string));
            break;
        case node_type::array_:
            new (&array) array_type(std::move(src.array));
            break;
        case node_type::object_:
            new (&object) object_type(std::move(src.object));
            break;
        }
        src.type = node_type::null_;
    }

    node& operator=(node const& src)
    {
        if (this == &src)
            return *this;

        type = src.type;
        switch (type) {
        case node_type::number_:
            number = src.number;
            break;
        case node_type::string_:
            new (&string) string_type(src.string);
            break;
        case node_type::array_:
            new (&array) array_type(src.array);
            break;
        case node_type::object_:
            new (&object) object_type(src.object);
            break;
        }
        return *this;
    }

    node& operator=(node&& src)
    {
        if (this == &src)
            return *this;

        type = src.type;
        switch (type) {
        case node_type::number_:
            number = src.number;
            break;
        case node_type::string_:
            new (&string) string_type(std::move(src.string));
            break;
        case node_type::array_:
            new (&array) array_type(std::move(src.array));
            break;
        case node_type::object_:
            new (&object) object_type(std::move(src.object));
            break;
        }
        src.type = node_type::null_;
        return *this;
    }

    ~node()
    {
        if (type == node_type::string_)
            string.~basic_string();
        if (type == node_type::array_)
            array.~vector();
        if (type == node_type::object_)
            object.clear();
    }

    node& operator[](int index)
    {
        // TODO: check whether the node type is array
        return array[index];
    }

    node& operator[](node::key_type key)
    {
        // TODO: check whether the node type is object
        return object[key];
    }

    node& at(int index)
    {
        // TODO: check whether the node type is array
        return array.at(index);
    }

    node& at(node::key_type key)
    {
        // TODO: check whether the node type is object
        return object.at(key);
    }


};

// since C++ 17 
node() -> node<void>;

int main()
{
    node mnode;
    mnode.type = node_type::object_;
    new (&mnode.object) node::object_type;

    node cnode;
    cnode.type = node_type::number_;
    cnode.number = 11;

    mnode.object.emplace("num", cnode);

    std::string key("nusm");
    auto& gnode = mnode.object["num"];
    std::cout << gnode.get<double>();
}