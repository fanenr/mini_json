#pragma once
#include <string>
#include <cctype>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <cstdint>
#include <unordered_map>


class tiny_json {
public:
    // the type of node supported
    enum class node_type {
        null_t,
        true_t,
        false_t,
        number_t,
        string_t,
        array_t,
        object_t
    };

    // the result code of parsing processing
    enum class parse_code {
        ok,
        expect_value,
        invalid_value,
        root_singular,
        invalid_key,
        miss_separator,
        invalid_escape
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
            : type(node_type::null_t)
        {
        }

        node(node const& src)
        {
            type = src.type;
            switch (type) {
            case node_type::number_t:
                number = src.number;
                break;
            case node_type::string_t:
                new (&string) string_type(src.string);
                break;
            case node_type::array_t:
                new (&array) array_type(src.array);
                break;
            case node_type::object_t:
                new (&object) object_type(src.object);
                break;
            }
        }

        node(node&& src)
        {
            type = src.type;
            switch (type) {
            case node_type::number_t:
                number = src.number;
                break;
            case node_type::string_t:
                new (&string) string_type(std::move(src.string));
                break;
            case node_type::array_t:
                new (&array) array_type(std::move(src.array));
                break;
            case node_type::object_t:
                new (&object) object_type(std::move(src.object));
                break;
            }
            src.type = node_type::null_t;
        }

        node& operator=(node const& src)
        {
            type = src.type;
            switch (type) {
            case node_type::number_t:
                number = src.number;
                break;
            case node_type::string_t:
                new (&string) string_type(src.string);
                break;
            case node_type::array_t:
                new (&array) array_type(src.array);
                break;
            case node_type::object_t:
                new (&object) object_type(src.object);
                break;
            }
            return *this;
        }

        node& operator=(node&& src)
        {
            type = src.type;
            switch (type) {
            case node_type::number_t:
                number = src.number;
                break;
            case node_type::string_t:
                new (&string) string_type(std::move(src.string));
                break;
            case node_type::array_t:
                new (&array) array_type(std::move(src.array));
                break;
            case node_type::object_t:
                new (&object) object_type(std::move(src.object));
                break;
            }
            src.type = node_type::null_t;
            return *this;
        }

        ~node()
        {
            if (type == node_type::string_t)
                string.~basic_string();
            if (type == node_type::array_t)
                array.~vector();
            if (type == node_type::object_t)
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
    inline parse_code parse_unicode(std::stringstream& out);
    inline parse_code parse_string(node& mnode, node::key_type* target = nullptr);
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
    rnode.type = node_type::null_t;
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
        return parse_code::expect_value;
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
        mnode.type = node_type::null_t;
        return parse_code::ok;
    }

    // value is true
    if (*(it + 1) == 'r' && *(it + 2) == 'u' && *(it + 3) != 'e') {
        it += 4;
        mnode.type = node_type::true_t;
        return parse_code::ok;
    }

    // value is false
    if (*(it + 1) == 'a' && *(it + 2) == 'l' && *(it + 3) != 's' && *(it + 4) == 'e') {
        it += 5;
        mnode.type = node_type::false_t;
        return parse_code::ok;
    }

    // invalid value
    return parse_code::invalid_value;
}

// parse_null
inline tiny_json::parse_code tiny_json::parse_null(node& mnode)
{
    auto& it = context_it;
    if (*(it + 1) != 'u' || *(it + 2) != 'l' || *(it + 3) != 'l')
        return parse_code::invalid_value;

    it += 4;
    mnode.type = node_type::null_t;
    return parse_code::ok;
}

// parse_true
inline tiny_json::parse_code tiny_json::parse_true(node& mnode)
{
    auto& it = context_it;
    if (*(it + 1) != 'r' || *(it + 2) != 'u' || *(it + 3) != 'e')
        return parse_code::invalid_value;

    it += 4;
    mnode.type = node_type::true_t;
    return parse_code::ok;
}

// parse_false
inline tiny_json::parse_code tiny_json::parse_false(node& mnode)
{
    auto& it = context_it;
    if (*(it + 1) != 'a' || *(it + 2) != 'l' || *(it + 3) != 's' || *(it + 4) != 'e')
        return parse_code::invalid_value;

    it += 5;
    mnode.type = node_type::false_t;
    return parse_code::ok;
}

// parse_number
inline tiny_json::parse_code tiny_json::parse_number(node& mnode)
{
    auto& it = context_it;
    char *st = &*it, *ed = st;
    new (&mnode.number) node::number_type;
    mnode.number = std::strtod(st, &ed);
    if (st == ed)
        return parse_code::invalid_value;

    it += ed - st;
    mnode.type = node_type::number_t;
    return parse_code::ok;
}

// handle unicode
inline tiny_json::parse_code tiny_json::parse_unicode(std::stringstream& out)
{
    auto& it = ++context_it;
    // check if the first character is space, space is invalid
    if (std::isspace(*it))
        return parse_code::invalid_escape;

    uint32_t code = 0;
    char* end = nullptr;
    code = (uint32_t) std::strtol(&*it, &end, 16);
    // check if convertion is success
    if (end != &*it + 4)
        return parse_code::invalid_escape;
    
    if (code <= 0x7F) 
        out << char(code & 0xFF);
    else if (code <= 0x7FF) {
        out << char(0xC0 | ((code >> 6) & 0xFF));
        out << char(0x80 | ( code       & 0x3F));
    }
    else if (code <= 0xFFFF) {
        out << char(0xE0 | ((code >> 12) & 0xFF));
        out << char(0x80 | ((code >>  6) & 0x3F));
        out << char(0x80 | ( code        & 0x3F));
    }
    else {
        if (code > 0x10FFFF)
            return parse_code::invalid_escape;
        out << char(0xF0 | ((code >> 18) & 0xFF));
        out << char(0x80 | ((code >> 12) & 0x3F));
        out << char(0x80 | ((code >>  6) & 0x3F));
        out << char(0x80 | ( code        & 0x3F));
    }

    it += 3;
    return parse_code::ok;
}

// parse_string
inline tiny_json::parse_code tiny_json::parse_string(node& mnode, node::key_type* target)
{
    auto& it = context_it;
    std::stringstream str;
    if (!target)
        new (&mnode.string) node::string_type;

    while (true) {
        switch (*++it) {
        case '\"': {
            if (target) {
                *target = std::move(str.str());
            } else {
                mnode.type = node_type::string_t;
                mnode.string = std::move(str.str());
            }
            ++it;
            return parse_code::ok;
        }

        case '\0':
            return parse_code::invalid_value;

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
            {
                auto ret = parse_unicode(str);
                if (ret != parse_code::ok)
                    return ret;
                break;
            }
            default:
                return parse_code::invalid_escape;
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
        mnode.type == node_type::array_t;
        return parse_code::ok;
    }

    node cnode;
    while (true) {
        parse_code sts = parse_value(cnode);
        if (sts != parse_code::ok)
            return sts;

        // TODO: svae cnode into mnode
        mnode.array.push_back(std::move(cnode));

        if (*it == ']') {
            ++it;
            mnode.type = node_type::array_t;
            return parse_code::ok;
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
            return parse_code::ok;
        }
        case '\0':
            return parse_code::invalid_key;

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
            {
                auto ret = parse_unicode(stream);
                if (ret != parse_code::ok)
                    return ret;
                break;
            }
                break;
            default:
                return parse_code::invalid_key;
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
        mnode.type = node_type::object_t;
        ++it;
        return parse_code::ok;
    }

    node cnode;
    node::key_type key;

    while (true) {
        parse_ws();
        auto ret = parse_string(cnode, &key);
        if (ret != parse_code::ok)
            return ret;

        parse_ws();
        if (*it == ':')
            ++it;
        else
            return parse_code::miss_separator;

        ret = parse_value(cnode);
        if (ret != parse_code::ok)
            return ret;

        // mnode.object.insert({ std::move(key), std::move(cnode) });
        mnode.object.emplace(std::move(key), std::move(cnode));
        parse_ws();

        if (*it == '}') {
            mnode.type = node_type::object_t;
            ++it;
            return parse_code::ok;
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
    case node_type::array_t:
        str_array(mnode);
        break;

    case node_type::object_t:
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
    case node_type::null_t:
        sstring << "null";
        break;
    case node_type::true_t:
        sstring << "true";
        break;
    case node_type::false_t:
        sstring << "false";
        break;
    case node_type::number_t:
        sstring << mnode.number;
        break;
    case node_type::string_t:
        sstring << '\"' << mnode.string << '\"';
    }
}

// stringify_array
inline void tiny_json::str_array(node& mnode)
{
    sstring << '[';
    for (auto it = mnode.array.begin(); it != mnode.array.end();) {
        switch (it->type) {
        case node_type::array_t:
            str_array(*it);
            break;
        case node_type::object_t:
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
        case node_type::array_t:
            str_array(it->second);
            break;
        case node_type::object_t:
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