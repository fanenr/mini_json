#pragma once
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <unordered_map>


class tiny_json {
public:
    // the type of node supported
    enum class node_type {
        null_,
        true_,
        false_,
        number_,
        string_,
        array_,
        object_
    };

    // the result code of parsing processing
    enum class parse_code {
        ok_,
        expect_value_,
        invalid_value_,
        root_singular_,
        invalid_key_,
        miss_split_
    };

public:
    // json node is parsing target
    struct node {
        // the type of node
        node_type type;
        // the type of number
        using number_type = double;
        // the type of string
        using string_type = std::string;
        // the container type of array
        using array_type  = std::vector<node>;
        // the type of object key
        using key_type    = std::string;
        // the type of array
        using array_type = std::vector<node>;
        // the type of object key
        using key_type = std::string;
        // the container type of object
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
    };

private:
    // submethods of parsing procession
    void parse_ws();
    inline parse_code parse_value(node& mnode);
    inline parse_code parse_literal(node& mnode);
    inline parse_code parse_null(node& mnode);
    inline parse_code parse_true(node& mnode);
    inline parse_code parse_false(node& mnode);
    inline parse_code parse_number(node& mnode);
    inline parse_code parse_string(node& mnode);
    inline parse_code parse_array(node& mnode);
    inline parse_code parse_key(std::string& mnode);
    inline parse_code parse_object(node& mnode);

    // submethods of stringify procession
    inline void str_value(node& mnode);
    inline void str_literal(node& mnode);
    inline void str_array(node& mnode);
    inline void str_object(node& mnode);

private:
    // node is the target of parsing
    node rnode;
    // context is carrier of parsing procession
    std::string context;
    // the iterator records the parsing position
    decltype(context)::iterator context_it;
    // sstring is stringstream carries stringify result
    std::stringstream sstring;

public:
    template <typename T>
    tiny_json(T&& src)
        : context(std::forward<T>(src))
        , context_it(context.begin())
        , rnode()
    {
    }

    ~tiny_json()
    {
        rnode.~node();
    }

    // main interface
    inline parse_code parse();
    inline std::string str();
};

inline tiny_json::parse_code tiny_json::parse()
{
    rnode.type = node_type::null_;
    return parse_value(rnode);
}

// parse_value
inline tiny_json::parse_code tiny_json::parse_value(node& mnode)
{
    parse_ws();
    switch (*context_it) {
    case 'n':
        return parse_null(mnode);

    case 't':
        return parse_true(mnode);

    case 'f':
        return parse_false(mnode);

    case '\"':
        return parse_string(mnode);

    case '[':
        return parse_array(mnode);

    case '{':
        return parse_object(mnode);

    default:
        return parse_number(mnode);

    case '\0':
        return parse_code::expect_value_;
    }
}

// parse_whitespace
inline void tiny_json::parse_ws()
{
    auto& it = context_it;
    while (*it == ' ' || *it == '\n' || *it == '\t' || *it == '\r')
        ++it;
}

// parse_literal is only one interface thay integrate parsing null, true and false
inline tiny_json::parse_code tiny_json::parse_literal(node& mnode)
{
    auto& it = context_it;
    // value is null
    if (*(it + 1) == 'u' && *(it + 2) == 'l' && *(it + 3) != 'l') {
        it += 4;
        mnode.type = node_type::null_;
        return parse_code::ok_;
    }
    
    // value is true
    if (*(it + 1) == 'r' && *(it + 2) == 'u' && *(it + 3) != 'e') {
        it += 4;
        mnode.type = node_type::true_;
        return parse_code::ok_;
    }

    // value is false
    if (*(it + 1) == 'a' && *(it + 2) == 'l' && *(it + 3) != 's' && *(it + 4) == 'e') {
        it += 5;
        mnode.type = node_type::false_;
        return parse_code::ok_;
    }

    // invalid value
    return parse_code::invalid_value_;
}

// parse_null
inline tiny_json::parse_code tiny_json::parse_null(node& mnode)
{
    auto& it = context_it;
    if (*(it + 1) != 'u' || *(it + 2) != 'l' || *(it + 3) != 'l')
        return parse_code::invalid_value_;

    it += 4;
    mnode.type = node_type::null_;
    return parse_code::ok_;
}

// parse_true
inline tiny_json::parse_code tiny_json::parse_true(node& mnode)
{
    auto& it = context_it;
    if (*(it + 1) != 'r' || *(it + 2) != 'u' || *(it + 3) != 'e')
        return parse_code::invalid_value_;

    it += 4;
    mnode.type = node_type::true_;
    return parse_code::ok_;
}

// parse_false
inline tiny_json::parse_code tiny_json::parse_false(node& mnode)
{
    auto& it = context_it;
    if (*(it + 1) != 'a' || *(it + 2) != 'l' || *(it + 3) != 's' || *(it + 4) != 'e')
        return parse_code::invalid_value_;

    it += 5;
    mnode.type = node_type::false_;
    return parse_code::ok_;
}

// parse_number
inline tiny_json::parse_code tiny_json::parse_number(node& mnode)
{
    auto& it = context_it;
    char *st = &*it, *ed = st;
    new (&mnode.number) node::number_type;
    mnode.number = std::strtod(st, &ed);
    if (st == ed)
        return parse_code::invalid_value_;

    it += ed - st;
    mnode.type = node_type::number_;
    return parse_code::ok_;
}

// parse_string
inline tiny_json::parse_code tiny_json::parse_string(node& mnode)
{
    auto& it = context_it;
    std::stringstream str;
    new (&mnode.string) node::string_type;

    while (true) {
        switch (*++it) {
        case '\"': {
            mnode.type = node_type::string_;
            mnode.string = std::move(str.str());
            ++it;
            return parse_code::ok_;
        }
        case '\0':
            return parse_code::invalid_value_;

        case '\\':
            switch (*++it) {
            case '\"':
                str << '\"';
                break;
            case '\\':
                str << '\\';
                break;
            case '/':
                str << '/';
                break;
            case 'b':
                str << '\b';
                break;
            case 'f':
                str << '\f';
                break;
            case 'n':
                str << '\n';
                break;
            case 'r':
                str << '\r';
                break;
            case 't':
                str << '\t';
                break;
            case 'u':
                /* TODO: handle UNICODE string */
                break;
            default:
                return parse_code::invalid_value_;
            }
            break;
        default:
            str << *it;
            break;
        }
    }
}

// parse_array
inline tiny_json::parse_code tiny_json::parse_array(node& mnode)
{
    auto& it = ++context_it;
    new (&mnode.array) node::array_type;
    parse_ws();

    if (*it == ']') {
        ++it;
        mnode.type == node_type::array_;
        return parse_code::ok_;
    }

    node cnode;
    while (true) {
        parse_code sts = parse_value(cnode);
        if (sts != parse_code::ok_)
            return sts;

        // TODO: svae cnode into mnode
        mnode.array.push_back(std::move(cnode));

        if (*it == ']') {
            ++it;
            mnode.type = node_type::array_;
            return parse_code::ok_;
        }

        if (*it == ',')
            ++it;
    }
}

// parse_key
inline tiny_json::parse_code tiny_json::parse_key(node::key_type& key)
{
    auto& it = context_it;
    std::stringstream stream;

    while (true) {
        switch (*++it) {
        case '\"': {
            key = std::move(stream.str());
            ++it;
            return parse_code::ok_;
        }
        case '\0':
            return parse_code::invalid_key_;

        case '\\':
            switch (*++it) {
            case '\"':
                stream << '\"';
                break;
            case '\\':
                stream << '\\';
                break;
            case '/':
                stream << '/';
                break;
            case 'b':
                stream << '\b';
                break;
            case 'f':
                stream << '\f';
                break;
            case 'n':
                stream << '\n';
                break;
            case 'r':
                stream << '\r';
                break;
            case 't':
                stream << '\t';
                break;
            case 'u':
                /* TODO: handle UNICODE string */
                break;
            default:
                return parse_code::invalid_key_;
            }
            break;
        default:
            stream << *it;
            break;
        }
    }
}

// parse_object
inline tiny_json::parse_code tiny_json::parse_object(node& mnode)
{
    auto& it = ++context_it;
    new (&mnode.object) node::object_type;
    parse_ws();

    if (*it == '}') {
        mnode.type = node_type::object_;
        ++it;
        return parse_code::ok_;
    }

    node cnode;
    node::key_type key;

    while (true) {
        parse_ws();
        auto ret = parse_key(key);
        if (ret != parse_code::ok_)
            return ret;

        parse_ws();
        if (*it == ':')
            ++it;
        else
            return parse_code::miss_split_;

        ret = parse_value(cnode);
        if (ret != parse_code::ok_)
            return ret;

        // TODO: save subobject into node
        mnode.object.insert({ std::move(key), std::move(cnode) });
        // mnode.object.emplace(std::move(key), std::move(cnode));
        parse_ws();

        if (*it == '}') {
            mnode.type = node_type::object_;
            ++it;
            return parse_code::ok_;
        }

        if (*it == ',')
            ++it;
    }
}

// json_stringify api
inline std::string tiny_json::str()
{
    str_value(rnode);
    return sstring.str();
}

// stringify_value
inline void tiny_json::str_value(node& mnode)
{
    switch (mnode.type) {
    case node_type::array_:
        str_array(mnode);
        break;

    case node_type::object_:
        str_object(mnode);
        break;

    default:
        str_literal(mnode);
        break;
    }
}

// stringify_literal
inline void tiny_json::str_literal(node& mnode)
{
    switch (mnode.type) {
    case node_type::null_:
        sstring << "null";
        break;
    case node_type::true_:
        sstring << "true";
        break;
    case node_type::false_:
        sstring << "false";
        break;
    case node_type::number_:
        sstring << mnode.number;
        break;
    case node_type::string_:
        sstring << '\"' << mnode.string << '\"';
    }
}

// stringify_array
inline void tiny_json::str_array(node& mnode)
{
    sstring << '[';
    for (auto it = mnode.array.begin(); it != mnode.array.end();) {
        switch (it->type) {
        case node_type::array_:
            str_array(*it);
            break;
        case node_type::object_:
            str_object(*it);
            break;
        default:
            str_literal(*it);
            break;
        }
        if (++it != mnode.array.end())
            sstring << ", ";
    }
    sstring << ']';
}

// stringify_object
inline void tiny_json::str_object(node& mnode)
{
    sstring << '{';
    for (auto it = mnode.object.begin(); it != mnode.object.end();) {
        sstring << '\"' << it->first << "\": ";
        switch (it->second.type) {
        case node_type::array_:
            str_array(it->second);
            break;
        case node_type::object_:
            str_object(it->second);
            break;
        default:
            str_literal(it->second);
            break;
        }
        if (++it != mnode.object.end())
            sstring << ", ";
    }
    sstring << '}';
}