#pragma once
#include <boost/optional.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mini_json {

// forward declaration
class node;

/**
 * node_type declare the supported type of json node.
 * These types are not exact type of node data.
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

/**
 * node is the main component of json
 * which represents a tree node of the json docmention
 * its content can be change in runtime with set method
 * also can get its value with get method
 */
class node {
private:
    node_t ntype;

private:
    std::variant<null_t, bool, number_t, string_t, array_t, object_t> data;

private:
    // node2n can convert node_t value to int during compilation
    static constexpr inline int node2n(node_t n) noexcept
    {
        return static_cast<int>(n);
    }

    // t2node return a node_t
    template <typename T>
    static constexpr inline node_t t2node() noexcept
    {
        if constexpr (std::is_same_v<T, null_t>)
            return node_t::null_t;
        else if (std::is_same_v<T, bool>)
            return node_t::bool_t;
        else if (std::is_integral_v<T> || std::is_floating_point_v<T>)
            return node_t::number_t;
        else if (std::is_constructible_v<string_t, T>)
            return node_t::string_t;
        else if (std::is_same_v<T, array_t>)
            return node_t::array_t;
        else if (std::is_same_v<T, object_t>)
            return node_t::object_t;
        else
            static_assert(true, "t2node got invalid value");
    }

    template <node_t N = node_t::null_t>
    struct node2t {
        using type = null_t;
    };

    /**
     * use simple TMP trick to get node's type
     */
    template <node_t N = node_t::null_t>
    using node2t_t = typename node2t<N>::type;

public:
    /**
     * unsepcific node is a node of null_t type
     * the argument will be forwarded to data
     * this function template will handle three action: construct, copy and move
     */
    template <typename T = null_t>
    explicit constexpr node(T&& init = nullptr)
    {
        using U = std::remove_const_t<std::remove_reference_t<T>>;
        if constexpr (!std::is_same_v<node, U>) {
            ntype = t2node<U>();
            if constexpr (t2node<U>() == node_t::number_t)
                data = static_cast<number_t>(init);
            else
                data = std::forward<T>(init);
        } else if (std::is_lvalue_reference_v<T>) {
            auto tmp = init.data;
            data.swap(tmp);
            ntype = init.ntype;
        } else {
            data.swap(init.data);
            ntype = init.ntype;
            init.data = null_t {};
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
        other.data = null_t {};
        return *this;
    }

    /**
     * a node is equal to another requires both their type and data is equal
     */
    bool operator==(node const& other) const
    {
        return ntype == other.ntype && data == other.data;
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
    std::conditional_t<node2n(N) < node2n(node_t::string_t), node2t_t<N>, node2t_t<N>&>
    get()
    {
        return std::get<node2t_t<N>>(data);
    }

    template <node_t N>
    std::conditional_t<node2n(N) < node2n(node_t::string_t), node2t_t<N>, node2t_t<N> const&>
    get() const
    {
        return std::get<node2t_t<N>>(data);
    }

    boost::optional<bool> get_bool() const noexcept
    {
        if (ntype == node_t::bool_t && data.index() == node2n(node_t::bool_t))
            return std::get<bool>(data);
        return {};
    }

    boost::optional<number_t> get_num() const noexcept
    {
        if (ntype == node_t::number_t && data.index() == node2n(node_t::number_t))
            return std::get<number_t>(data);
        return {};
    }

    boost::optional<string_t&> get_str() noexcept
    {
        if (ntype == node_t::string_t && data.index() == node2n(node_t::string_t))
            return std::get<string_t>(data);
        return {};
    }

    boost::optional<array_t const&> get_arr() const noexcept
    {
        if (ntype == node_t::array_t && data.index() == node2n(node_t::array_t))
            return std::get<array_t>(data);
        return {};
    }

    boost::optional<object_t const&> get_obj() const noexcept
    {
        if (ntype == node_t::object_t && data.index() == node2n(node_t::object_t))
            return std::get<object_t>(data);
        return {};
    }

    boost::optional<string_t const&> get_str() const noexcept
    {
        if (ntype == node_t::string_t && data.index() == node2n(node_t::string_t))
            return std::get<string_t>(data);
        return {};
    }

    boost::optional<array_t&> get_arr() noexcept
    {
        if (ntype == node_t::array_t && data.index() == node2n(node_t::array_t))
            return std::get<array_t>(data);
        return {};
    }

    boost::optional<object_t&> get_obj() noexcept
    {
        if (ntype == node_t::object_t && data.index() == node2n(node_t::object_t))
            return std::get<object_t>(data);
        return {};
    }

    // check if a node is not empty
    operator bool() const noexcept
    {
        return ntype != node_t::null_t;
    }

    /**
     * type method return the current type of node
     */
    node_t type() const noexcept
    {
        return ntype;
    }
};

/**
 * there are some specialization
 */
template <>
struct node::node2t<node_t::bool_t> {
    using type = bool;
};

template <>
struct node::node2t<node_t::number_t> {
    using type = number_t;
};

template <>
struct node::node2t<node_t::string_t> {
    using type = string_t;
};

template <>
struct node::node2t<node_t::array_t> {
    using type = array_t;
};

template <>
struct node::node2t<node_t::object_t> {
    using type = object_t;
};

} // namespace mini_json