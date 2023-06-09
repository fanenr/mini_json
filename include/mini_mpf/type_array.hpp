#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace mini_mpf {

template <typename... Types>
class type_array {

private:
    constexpr static std::size_t length = sizeof...(Types);

    template <std::size_t Index, typename Arr>
    struct _at;

    template <typename T, typename... Others>
    struct _at<0, type_array<T, Others...>> {
        using type = T;
    };

    template <std::size_t Index, typename T, typename... Others>
    struct _at<Index, type_array<T, Others...>> {
        using type = typename _at<Index - 1, type_array<Others...>>::type;
    };

    template <template <typename...> typename T>
    struct _forward {
        using type = T<Types...>;
    };

public:
    // type of self
    using self = type_array<Types...>;

    // at is used to use a type at Pos in an array
    template <std::size_t Index>
    using at = typename _at<Index, self>::type;

    // len will return the number of types in an array
    constexpr static std::size_t len() noexcept
    {
        return length;
    }

    // cmp is used to compare a given type with the type at Pos in an array
    template <typename T, std::size_t Pos>
    constexpr static bool cmp() noexcept
    {
        return std::is_same_v<at<Pos>, T>;
    }

    // forward can forward all types to any suitable templates
    template <template <typename...> typename T>
    using forward = typename _forward<T>::type;

    // find will return Index associated with the given type or assert failed
    template <typename Type, std::size_t Pos = 0>
    constexpr static std::size_t find() noexcept
    {
        constexpr bool ret = cmp<Type, Pos>();

        if constexpr (ret)
            return Pos;

        if constexpr (Pos + 1 < length)
            return find<Type, Pos + 1>();

        if constexpr (Pos == length - 1)
            static_assert(ret, "the given type not exists");
    }

    // find_if will return an optional
    template <typename Type, std::size_t Pos = 0>
    constexpr static std::optional<std::size_t> find_if() noexcept
    {
        constexpr bool ret = cmp<Type, Pos>();

        if constexpr (ret)
            return Pos;

        if constexpr (!ret && Pos + 1 < length)
            return find_if<Type, Pos + 1>();

        if constexpr (!ret && Pos == length - 1)
            return std::nullopt;
    }

    // for_each is used to traverse all types in an array and perform some actions
    // Func only receive the current type in array
    template <template <typename> typename Func, std::size_t Pos = 0, typename... Args>
    constexpr static void for_each(Args&&... args)
    {
        if constexpr (Pos < length)
            std::invoke(Func<at<Pos>>(), std::forward<Args>(args)...);

        if constexpr (Pos + 1 < length)
            for_each<Func, Pos + 1>(std::forward<Args>(args)...);
    }

    // Func receive both the current type and index in array
    template <template <typename, std::size_t> typename Func, std::size_t Pos = 0, typename... Args>
    constexpr static void for_each(Args&&... args)
    {
        if constexpr (Pos < length)
            std::invoke(Func<at<Pos>, Pos>(), std::forward<Args>(args)...);

        if constexpr (Pos + 1 < length)
            for_each<Func, Pos + 1>(std::forward<Args>(args)...);
    }
};

};