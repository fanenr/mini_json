#pragma once
#include "node.hpp"
#include <memory>
#include <string>

namespace mini_json {

/**
 * json provides the parsing and stringing manipulation
 */
class json {

public:
    /**
     * error_code includes all types of error while parsing and stringing
     */
    enum class error_code {
        non,
        invalid_key,
        expect_value,
        root_singular,
        invalid_value,
        miss_separator,
        invalid_escape,
    };

private:
    std::unique_ptr<node> root = nullptr;
    std::unique_ptr<std::string> string = nullptr;
    std::string context;
    std::string::iterator context_it;
    error_code perr = error_code::non;
    error_code serr = error_code::non;

public:
    /**
     * json accept an context while construcing
     * which could copy or move from argument
     */
    json(std::string init)
        : context(std::move(init))
        , context_it(context.begin())
    {
    }

    /**
     * parse operation will try to parse the context to root node
     * which should return an optional
     * the optional maybe hold a shared_ptr pointed to root node
     */
    node* parse()
    {
        if (!root)
            root = std::make_unique<node>();

        if (root && parse_value(*root))
            return root.get();

        root = nullptr;
        return nullptr;
    }

    /**
     * str operation will try to stringify the root node to string
     * which should also return an optional
     * the optional maybe hold a shared_ptr pointed to string
     */
    std::string* str()
    {
        if (!string)
            string = std::make_unique<std::string>();

        if (root && str_value(*root))
            return string.get();

        string = nullptr;
        return nullptr;
    }

    /**
     * get error code
     */
    error_code errp() const noexcept
    {
        return perr;
    }

    error_code errs() const noexcept
    {
        return serr;
    }

private:
    /**
     * submethods about parsing
     */
    bool parse_unicode(std::string& out);
    bool parse_key(std::string& str);
    bool parse_literal(node& mnode);
    bool parse_object(node& mnode);
    bool parse_string(node& mnode);
    bool parse_number(node& mnode);
    bool parse_value(node& mnode);
    bool parse_array(node& mnode);
    void parse_ws();

    // submethods about stringing
    std::string str_string(std::string_view src);
    bool str_literal(node& mnode);
    bool str_object(node& mnode);
    bool str_value(node& mnode);
    bool str_array(node& mnode);
};

/**
 * parse_value take charge of distinguish the type of subnode
 * and dispatching the parsing tasks to other submethods
 */
inline bool json::parse_value(node& mnode)
{
    parse_ws();
    switch (*context_it) {
    case 'n':
        return parse_literal(mnode);

    case 't':
        return parse_literal(mnode);

    case 'f':
        return parse_literal(mnode);

    case '\"':
        return parse_string(mnode);

    case '[':
        return parse_array(mnode);

    case '{':
        return parse_object(mnode);

    default:
        return parse_number(mnode);

    case '\0':
        perr = error_code::expect_value;
        return false;
    }
}

/**
 * parse_ws let iterator point to next non-empty charactor
 */
inline void json::parse_ws()
{
    auto& it = context_it;
    while (*it == ' ' || *it == '\n' || *it == '\t' || *it == '\r')
        ++it;
}

/**
 * parse_literal take charge of parsing literal value
 * which includes null and bool
 */
inline bool json::parse_literal(node& mnode)
{
    auto& it = context_it;

    // value is null
    if (*(it + 1) == 'u' && *(it + 2) == 'l' && *(it + 3) == 'l') {
        it += 4;
        mnode.assign(nullptr);
        return true;
    }

    // value is true
    if (*(it + 1) == 'r' && *(it + 2) == 'u' && *(it + 3) == 'e') {
        it += 4;
        mnode.assign(true);
        return true;
    }

    // value is false
    if (*(it + 1) == 'a' && *(it + 2) == 'l' && *(it + 3) == 's' && *(it + 4) == 'e') {
        it += 5;
        mnode.assign(false);
        return true;
    }

    // invalid value
    perr = error_code::invalid_value;
    return false;
}

/**
 * parse_number take care of parsing the number literal
 * which is supported by standard function :)
 */
inline bool json::parse_number(node& mnode)
{
    auto& it = context_it;
    char *st = &*it, *ed = st;

    double num = static_cast<double>(std::strtod(st, &ed));
    if (st == ed) {
        perr = error_code::invalid_value;
        return false;
    }

    it += ed - st;
    mnode.assign(num);
    return true;
}

/**
 * parse_unicode is a submethod of parse_string and parse_key
 * which only support to convert the unicode to UTF-8
 */
inline bool json::parse_unicode(std::string& out)
{
    auto& it = ++context_it;
    // check whether the first character is space, space is invalid
    if (std::isspace(*it)) {
        perr = error_code::invalid_escape;
        return false;
    }

    uint32_t code = 0;
    char* end = nullptr;
    code = (uint32_t)std::strtol(&*it, &end, 16);
    // check if convertion is success
    if (end != &*it + 4) {
        perr = error_code::invalid_escape;
        return false;
    }

    char tmp[4] = { 0 };
    std::size_t len = 0;
    if (code <= 0x7F) {
        tmp[0] = char(code & 0xFF);
        len = 1;
    } else if (code <= 0x7FF) {
        tmp[0] = char(0xC0 | ((code >> 6) & 0xFF));
        tmp[1] = char(0x80 | (code & 0x3F));
        len = 2;
    } else if (code <= 0xFFFF) {
        tmp[0] = char(0xE0 | ((code >> 12) & 0xFF));
        tmp[1] = char(0x80 | ((code >> 6) & 0x3F));
        tmp[2] = char(0x80 | (code & 0x3F));
        len = 3;
    } else if (code <= 0x10FFFF) {
        tmp[0] = char(0xF0 | ((code >> 18) & 0xFF));
        tmp[1] = char(0x80 | ((code >> 12) & 0x3F));
        tmp[2] = char(0x80 | ((code >> 6) & 0x3F));
        tmp[3] = char(0x80 | (code & 0x3F));
        len = 4;
    } else {
        perr = error_code::invalid_escape;
        return false;
    }

    out.append(tmp, len);
    it += 3;
    return true;
}

/**
 * parse_string support parsing escape charactor and unicode
 * but it only support to parse to UTF-8 charactors
 */
inline bool json::parse_string(node& mnode)
{
    auto& it = context_it;
    std::string rlt;

    while (true) {
        switch (*++it) {
        case '\"': {
            mnode.assign(std::move(rlt));
            ++it;
            return true;
        }

        case '\0':
            perr = error_code::invalid_value;
            return false;

        case '\\':
            switch (*++it) {
            case '\"':
                rlt.append("\"");
                break;
            case '\\':
                rlt.append("\\");
                break;
            case '/':
                rlt.append("/");
                break;
            case 'b':
                rlt.append("\b");
                break;
            case 'f':
                rlt.append("\f");
                break;
            case 'n':
                rlt.append("\n");
                break;
            case 'r':
                rlt.append("\r");
                break;
            case 't':
                rlt.append("\t");
                break;
            case 'u': {
                std::string ustr;
                if (!parse_unicode(ustr))
                    return false;
                rlt.append(ustr);
                break;
            }
            default:
                perr = error_code::invalid_escape;
                return false;
            }
            break;

        default:
            rlt.append(&*it, 1);
            break;
        }
    }
}

/**
 * parse_array take charge of parsing array datastruture
 * which use vector as default container
 */
inline bool json::parse_array(node& mnode)
{
    auto& it = ++context_it;
    parse_ws();
    mnode.assign(node::arr_t());

    if (*it == ']') {
        ++it;
        return true;
    }

    node cnode;
    auto& arr = mnode.get<node::arr_t>();

    while (true) {
        if (!parse_value(cnode))
            return false;

        arr.push_back(std::move(cnode));

        parse_ws();
        if (*it == ']') {
            ++it;
            return true;
        }

        if (*it == ',')
            ++it;
    }
}

/**
 * parse_key is a submethod of parse_object
 * object use a string as its key
 */
inline bool json::parse_key(std::string& key)
{
    auto& it = context_it;

    while (true) {
        switch (*++it) {
        case '\"': {
            ++it;
            return true;
        }

        case '\0':
            perr = error_code::invalid_value;
            return false;

        case '\\':
            switch (*++it) {
            case '\"':
                key.append("\"");
                break;
            case '\\':
                key.append("\\");
                break;
            case '/':
                key.append("/");
                break;
            case 'b':
                key.append("\b");
                break;
            case 'f':
                key.append("\f");
                break;
            case 'n':
                key.append("\n");
                break;
            case 'r':
                key.append("\r");
                break;
            case 't':
                key.append("\t");
                break;
            case 'u': {
                std::string ustr;
                if (!parse_unicode(ustr))
                    return false;
                key.append(ustr);
                break;
            }
            default:
                perr = error_code::invalid_escape;
                return false;
            }
            break;

        default:
            key.append(&*it, 1);
            break;
        }
    }
}

/**
 * parse_object take charge of parsing object datastructure
 * object node use unordered_map as its default container
 */
inline bool json::parse_object(node& mnode)
{
    auto& it = ++context_it;
    parse_ws();
    mnode.assign(node::obj_t());

    if (*it == '}') {
        ++it;
        return true;
    }

    auto& obj = mnode.get<node::obj_t>();
    node cnode;
    std::string key;

    while (true) {
        parse_ws();
        if (!parse_key(key))
            return false;

        parse_ws();
        if (*it == ':') {
            ++it;
        } else {
            perr = error_code::miss_separator;
            return false;
        }

        if (!parse_value(cnode))
            return false;

        obj.emplace(std::move(key), std::move(cnode));

        parse_ws();
        if (*it == '}') {
            ++it;
            return true;
        }

        if (*it == ',')
            ++it;
    }
}

/**
 * str_value is the interface to stringify root node
 */
inline bool json::str_value(node& mnode)
{
    switch (mnode.type()) {
    case node::data_k::array:
        return str_array(mnode);

    case node::data_k::object:
        return str_object(mnode);

    default:
        return str_literal(mnode);
    }
    return false;
}

/**
 * because of escape charactors
 * node of string type need to be sepcially handled
 */
inline std::string json::str_string(std::string_view src)
{
    std::string ret;

    for (auto it = src.begin(); it != src.end(); ++it)
        switch (*it) {
        case '\"':
            ret.append("\\\"");
            break;

        default:
            ret.append(&*it, 1);
            break;
        }

    return ret;
}

/**
 * the interface of stringing literal node
 */
inline bool json::str_literal(node& mnode)
{
    switch (mnode.type()) {
    case node::data_k::null:
        string->append("null");
        break;

    case node::data_k::boolean:
        string->append(mnode.get<bool>() ? "true" : "false");
        break;

    case node::data_k::number: {
        auto conv = std::to_string(mnode.get<node::num_t>());
        string->append(std::string(conv.begin(), conv.end()));
        break;
    }

    case node::data_k::string:
        string->append("\"")
            .append(str_string(mnode.get<node::str_t>()))
            .append("\"");
        break;

    default:
        return false;
    }

    return true;
}

/**
 * stringing node of array type
 */
inline bool json::str_array(node& mnode)
{
    string->append("[");
    auto& arr = mnode.get<node::arr_t>();

    bool sts = false;
    for (auto it = arr.begin(); it != arr.end();) {

        switch (it->type()) {
        case node::data_k::array:
            sts = str_array(*it);
            break;

        case node::data_k::object:
            sts = str_object(*it);
            break;

        default:
            sts = str_literal(*it);
            break;
        }

        if (!sts)
            return false;

        if (++it != arr.end())
            string->append(", ");
    }

    string->append("]");
    return true;
}

/**
 * stringing node of object node
 */
inline bool json::str_object(node& mnode)
{
    string->append("{");
    auto& map = mnode.get<node::obj_t>();

    bool sts = false;
    for (auto it = map.begin(); it != map.end();) {
        string->append("\"")
            .append(str_string(it->first))
            .append("\": ");

        switch (it->second.type()) {
        case node::data_k::array:
            sts = str_array(it->second);
            break;

        case node::data_k::object:
            sts = str_object(it->second);
            break;

        default:
            sts = str_literal(it->second);
            break;
        }

        if (!sts)
            return false;

        if (++it != map.end())
            string->append(", ");
    }

    string->append("}");
    return true;
}

}; // namespace mini_json