#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mini_json {

// forward declaration
class node;

/**
 * node_type declare the supported type of json node.
 * These types are not real type of node data.
 * But in practical use, we use these types to distinguish
 */
enum class node_t {
    null_t,
    bool_t,
    number_t,
    string_t,
    array_t,
    object_t
};

/**
 * these type alias  will be used to replace the data type
 */

// type of null
using null_t = std::nullptr_t;

// type of number
using number_t = double;

/**
 * type of string, only support utf-8 string
 * since C++20, we should use std::u8string to replace it
 */
using string_t = std::string;

/**
 * array type should be a container
 * which should inlcude basic methods like push_back and emplace_back if it be replaced
 */
using array_t = std::vector<node>;

/**
 * key type as the key of object container, which should be a string type
 * for convenience, use string_t to represent it
 */
using key_t = string_t;

/**
 * object type should be a container
 * which should include basic methods like emplace and insert if it be replaced
 */
using object_t = std::unordered_map<key_t, node>;

// node2n can convert node_type value to int during compilation
static constexpr inline int node2n(node_t n) noexcept
{
    return static_cast<int>(n);
}

/**
 * data_v is a class template which used to deduce the type of data
 * use it during compilation with typename keyword
 */
template <node_t N>
struct data_v {
    using type = std::nullptr_t;
};

/**
 * since C++14, data_v can be simplified with alias template
 * now use data_t without typename keyword
 */
template <node_t N>
using data_t = typename data_v<N>::type;

/**
 * t2node is the inverse function to node2t
 * which can get a node_type during compilation
 */
template <typename T, int N = 0>
static constexpr inline node_t t2node() noexcept
{
    static_assert(N <= node2n(node_t::object_t), "t2node got invalid value");

    if constexpr (std::is_same_v<T, data_t<static_cast<node_t>(N)>>)
        return static_cast<node_t>(N);
    else
        return t2node<T, N + 1>();
}

/**
 * node is the main component of json
 * which represents a tree ndoe of the json docmention
 * its content can be change in runtime with set method
 * also can get its value with get method
 */
class node {
private:
    node_t ntype;

private:
    std::variant<null_t, bool, number_t, array_t, string_t, object_t> data;

public:
    /**
     * unsepcific node is a node of null_t type
     * the argument will be forwarded to data
     * this function template will handle three action: construct, copy and move
     */
    template <typename T = null_t>
    node(T&& init = nullptr)
    {
        using U = std::remove_const_t<std::remove_reference_t<T>>;
        if constexpr (!std::is_same_v<node, U>) {
            ntype = t2node<U>();
            data = std::forward<T>(init);
        } else if (std::is_lvalue_reference_v<T>) {
            auto tmp = init.data;
            data.swap(tmp);
            ntype = init.ntype;
        } else {
            data.swap(init.data);
            ntype = init.ntype;
            init.data = nullptr;
            init.ntype = node_t::null_t;
        }
    }

    /**
     * the two methods will not be used but needed
     */
    node(node const&) = default;
    node(node&&) = default;

    /**
     * overload assignment operator
     * support coping or moving from another node
     */
    node& operator=(node const& other)
    {
        ntype = other.ntype;
        auto tmp = other.data;
        data.swap(tmp);
        return *this;
    }

    node& operator=(node&& other)
    {
        ntype = other.ntype;
        data.swap(other.data);
        other.ntype = node_t::null_t;
        other.data = nullptr;
        return *this;
    }

    /**
     * set method used to reset content of node
     */
    template <typename T>
    void set(T&& src)
    {
        ntype = t2node<std::remove_const_t<std::remove_reference_t<T>>>();
        data = std::forward<T>(src);
    }

    /**
     * get method used to get value from data
     * it shuld return a reference if N is not the basic type
     */
    template <node_t N>
    std::conditional_t<node2n(N) < node2n(node_t::string_t), data_t<N>, data_t<N>&> get()
    {
        return std::get<data_t<N>>(data);
    }

    /**
     * type method return the current type of node
     */
    node_t type() noexcept
    {
        return ntype;
    }
};

/**
 * there are some sepcific for data_v template
 */
template <>
struct data_v<node_t::bool_t> {
    using type = bool;
};

template <>
struct data_v<node_t::number_t> {
    using type = number_t;
};

template <>
struct data_v<node_t::string_t> {
    using type = string_t;
};

template <>
struct data_v<node_t::array_t> {
    using type = array_t;
};

template <>
struct data_v<node_t::object_t> {
    using type = object_t;
};

} // namespace mini_json