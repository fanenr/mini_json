#pragma once
#include "../mini_mpf/type_umap.hpp"
#include "exception.hpp"
#include <array>
#include <cstddef>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace mini_json {

class node {

private:
    template <typename T>
    constexpr static bool is_num = std::is_floating_point_v<T> || std::is_integral_v<T>;

    template <typename Tar, typename Src>
    constexpr static bool convable = std::is_constructible_v<Tar, Src>;

    template <typename T1, typename T2>
    constexpr static bool is_same = std::is_same_v<T1, T2>;

    using obj_t = std::unordered_map<std::string, node>;
    using arr_t = std::vector<node>;
    using nil_t = std::nullptr_t;
    using str_t = std::string;
    using num_t = double;

public:
    friend class json;

    enum class data_k {
        null,
        array,
        object,
        string,
        number,
        boolean,
    };

    using data_t = mini_mpf::type_umap<data_k,
        nil_t,
        arr_t,
        obj_t,
        str_t,
        num_t,
        bool>;

private:
    data_t::forward<std::variant> data;

    data_k type()
    {
        return static_cast<data_k>(data.index());
    }

public:
    template <typename T>
    constexpr void assign(T&& elem)
    {
        using Pure = std::decay_t<T>;
        if constexpr (data_t::find_if<T>()) {
            data = std::forward<T>(elem);
        } else {
            if constexpr (is_same<nil_t, Pure>) {
                data = nil_t {};
            } else if constexpr (is_same<bool, Pure>) {
                data = elem;
            } else if constexpr (is_num<Pure>) {
                data = data_t::at<data_k::number>(elem);
            } else if constexpr (convable<str_t, T>) {
                data = str_t(std::forward<T>(elem));
            } else if constexpr (convable<arr_t, T>) {
                data = arr_t(std::forward<T>(elem));
            } else if constexpr (convable<obj_t, T>) {
                data = obj_t(std::forward<T>(elem));
            } else {
                throw bad_assign();
            }
        }
    }

    template <typename T>
    constexpr T& get()
    {
        using Pure = std::decay_t<T>;
        static_assert(data_t::find_if<Pure>(), "mini_json::node::get : invalid type");

        if (T* got = std::get_if<Pure>(&data); got)
            return *got;

        throw bad_get();
    }

    template <typename T>
    constexpr T const& get() const
    {
        using Pure = std::decay_t<T>;
        static_assert(data_t::find_if<Pure>(), "mini_json::node::get : invalid type");

        if (T const* got = std::get_if<Pure>(&data); got)
            return *got;

        throw bad_get();
    }

#define CHECK_AND_HANDLE(type)                               \
    if constexpr (convable<T, type>)                         \
        if (auto const* got = std::get_if<type>(&data); got) \
    return Pure(*got)

    template <typename T>
    T as() const
    {
        using Pure = std::decay_t<T>;

        CHECK_AND_HANDLE(nil_t);
        CHECK_AND_HANDLE(arr_t);
        CHECK_AND_HANDLE(obj_t);
        CHECK_AND_HANDLE(str_t);
        CHECK_AND_HANDLE(num_t);
        CHECK_AND_HANDLE(bool);

        throw bad_as();
    }

#undef CHECK_AND_HANDLE

public:
    template <typename T = std::nullptr_t>
    node(T&& val = T {})
    {
        assign(std::forward<T>(val));
    }

    node(node& src)
    {
        auto tmp = src.data;
        data.swap(tmp);
    }

    node(node const& src)
    {
        auto tmp = src.data;
        data.swap(tmp);
    }

    node(node&& src)
    {
        data.swap(src.data);
        src.data = nullptr;
    }

    node& operator=(node const& src)
    {
        if (this == &src)
            return *this;

        auto tmp = src.data;
        data.swap(tmp);
        return *this;
    }

    node& operator=(node&& src)
    {
        if (this == &src)
            return *this;

        data.swap(src.data);
        src.data = nullptr;
        return *this;
    }
}; // class node

}; // namespace mini_json