#pragma once
#include "exception"
#include "mini_json/exception.hpp"
#include <array>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mini_json {

class node {

private:
    friend class json;
    constexpr static std::size_t N = 7;

    std::variant<bool, double, std::nullptr_t, std::string,
        std::vector<node>, std::unordered_map<std::string, node>>
        data;

    template <std::size_t U>
    struct types {
        using type = std::decay_t<decltype(std::get<U>(data))>;
    };

    template <std::size_t U>
    using types_t = typename types<U>::type;

private:
    enum class result : char {
        same = 1,
        bad = 0,
        ok = 2
    };

    template <typename Exter>
    struct type_check_in {
        using R = result;
        using E = std::decay_t<Exter>;
        constexpr static R ret[N] {
            std::is_same_v<bool, E> ? R::same : R::bad,

            std::is_integral_v<E> || std::is_floating_point_v<E> ? R::ok : R::bad,

            std::is_same_v<types_t<2>, E> ? R::same : R::bad,

            std::is_constructible_v<types_t<3>, Exter> ? (std::is_same_v<types_t<3>, E> ? R::same : R::ok) : R::bad,

            std::is_constructible_v<types_t<4>, Exter> ? (std::is_same_v<types_t<4>, E> ? R::same : R::ok) : R::bad,

            std::is_constructible_v<types_t<5>, Exter> ? (std::is_same_v<types_t<5>, E> ? R::same : R::ok) : R::bad,
        };
    };

    template <typename E>
    constexpr static void pure_check()
    {
        static_assert(!std::is_reference_v<E>, "reference is invalid");
        static_assert(!std::is_pointer_v<E>, "pointer is invalid");
        static_assert(!std::is_const_v<E>, "constant is invalid");
    };

    template <typename E, std::size_t U>
    constexpr static result check_ex_v()
    {
        return std::is_constructible_v<E, types_t<U>> ? (std::is_same_v<types_t<U>, E> ? result::same : result::ok) : result::bad;
    }

    template <typename Exter>
    struct type_check_ex {
        using R = result;
        using E = std::decay_t<Exter>;
        constexpr static R ret[N] {
            check_ex_v<E, 0>(),

            check_ex_v<E, 1>(),

            check_ex_v<E, 2>(),

            check_ex_v<E, 3>(),

            check_ex_v<E, 4>(),

            check_ex_v<E, 5>()
        };
    };

public:
    template <typename T, std::size_t Pos = 0>
    constexpr void assign(T&& elem)
    {
        static_assert(Pos < N, "mini_json::node::assign : invalid type");

        using Type_Check = type_check_in<T>;
        constexpr auto ret = Type_Check::ret[Pos];

        if constexpr (ret == Type_Check::R::same)
            data = std::forward<T>(elem);

        if constexpr (ret == Type_Check::R::ok)
            data = types_t<Pos>(std::forward<T>(elem));

        if constexpr (ret == Type_Check::R::bad)
            assign<T, Pos + 1>(std::forward<T>(elem));
    }

    template <typename T, std::size_t Pos = 0>
    constexpr T& get()
    {
        static_assert(Pos < N, "mini_json::node::get : invalid type");
        pure_check<T>();

        if constexpr (!std::is_same_v<T, types_t<Pos>>)
            return get<T, Pos + 1>();

        if (T* got = std::get_if<T>(&data); got)
            return *got;

        throw bad_get();
    }

    template <typename T, std::size_t Pos = 0>
    constexpr T const& get() const
    {
        static_assert(Pos < N, "mini_json::node::get(const) : invalid type");
        pure_check<T>();

        if constexpr (!std::is_same_v<T, types_t<Pos>>)
            return get<T, Pos + 1>();

        if (T const* got = std::get_if<T>(&data); got)
            return *got;

        throw bad_get();
    }

    template <typename T, std::size_t Pos = 0>
    T as() const
    {
        static_assert(Pos < N, "mini_json::node::as : invalid type");
        pure_check<T>();

        using Type_Check = type_check_ex<T>;
        constexpr auto ret = Type_Check::ret[Pos];

        if constexpr (ret == Type_Check::R::same)
            if (types_t<Pos> const* got = std::get_if<Pos>(&data); got)
                return *got;

        if constexpr (ret == Type_Check::R::ok)
            if (types_t<Pos> const* got = std::get_if<Pos>(&data); got)
                return T(*got);

        if constexpr (Pos < N - 1)
            return as<T, Pos + 1>();

        throw bad_as();
    }

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