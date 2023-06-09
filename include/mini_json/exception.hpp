#pragma once
#include <exception>

namespace mini_json {

class bad_assign : public std::exception {
public:
    char const* what() const noexcept override
    {
        return "the given type is invalid for node data";
    }
};

class bad_get : public std::exception {
public:
    char const* what() const noexcept override
    {
        return "node does not hold value of expected type";
    }
};

class bad_as : public std::exception {
public:
    char const* what() const noexcept override
    {
        return "data of node cannot be converted to expected type";
    }
};

};