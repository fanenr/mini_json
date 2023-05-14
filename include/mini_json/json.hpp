#include "node.hpp"
#include <memory>
#include <optional>
#include <sstream>

namespace mini_json {

/**
 * context_t is the type of context which holds the json info
 * output_t is the type of stringify result 
 */
using context_t = std::string;
using output_t = std::string;

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
        expect_value,
        invalid_value,
        root_singular,
        invalid_key,
        miss_separator,
        invalid_escape
    };

private:
    std::shared_ptr<node> root = nullptr;
    std::shared_ptr<output_t> string = nullptr;
    context_t context;
    context_t::iterator context_it;
    error_code perr = error_code::non;
    error_code serr = error_code::non;

public:
    /**
     * json accept an context while construcing
     * which could copy or move from argument 
     */
    json(context_t const& init)
        : context(init)
        , context_it(context.begin())
    {
    }

    json(context_t&& init)
        : context(std::move(init))
        , context_it(context.begin())
    {
    }

    /**
     * parse operation will try to parse the context to root node
     * which should return an optional
     * the optional maybe hold a shared_ptr pointed to root node 
     */
    std::optional<decltype(root)> parse()
    {
        if (!root)
         root = std::make_shared<node>();

        if (root && parse_value(*root))
            return root;
        root->set(nullptr);
        return {};
    }

    /**
     * str operation will try to stringify the root node to string
     * which should also return an optional
     * the optional maybe hold a shared_ptr pointed to string
     */
    std::optional<decltype(string)> str()
    {
        if (!string)
            string = std::make_shared<output_t>();

        if (root && str_value(*root))
            return string;
        string->clear();
        return {};
    }

private:
    /**
     * submethods about parsing 
     */
    void parse_ws();
    bool parse_value(node& mnode);
    bool parse_literal(node& mnode);
    bool parse_number(node& mnode);
    bool parse_unicode(string_t& out);
    bool parse_string(node& mnode);
    bool parse_array(node& mnode);
    bool parse_key(key_t& str);
    bool parse_object(node& mnode);

    // submethods about stringing
    bool str_value(node& mnode);
    output_t str_string(output_t const& src);
    bool str_literal(node& mnode);
    bool str_array(node& mnode);
    bool str_object(node& mnode);
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
        mnode.set(nullptr);
        return true;
    }

    // value is true
    if (*(it + 1) == 'r' && *(it + 2) == 'u' && *(it + 3) == 'e') {
        it += 4;
        mnode.set(true);
        return true;
    }

    // value is false
    if (*(it + 1) == 'a' && *(it + 2) == 'l' && *(it + 3) == 's' && *(it + 4) == 'e') {
        it += 5;
        mnode.set(false);
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
    
    number_t num = static_cast<number_t>(std::strtod(st, &ed));
    if (st == ed) {
        perr = error_code::invalid_value;
        return false;
    }
    
    it += ed - st;
    mnode.set(num);
    return true;
}

/**
 * parse_unicode is a submethod of parse_string and parse_key
 * which only support to convert the unicode to UTF-8
 */
inline bool json::parse_unicode(string_t& out)
{
    auto& it = ++context_it;
    // check whether the first character is space, space is invalid
    if (std::isspace(*it)) {
        perr = error_code::invalid_escape;
        return false;
    }

    uint32_t code = 0;
    char* end = nullptr;
    code = (uint32_t) std::strtol(&*it, &end, 16);
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
        tmp[1] = char(0x80 | ( code       & 0x3F));
        len = 2;
    } else if (code <= 0xFFFF) {
        tmp[0] = char(0xE0 | ((code >> 12) & 0xFF));
        tmp[1] = char(0x80 | ((code >>  6) & 0x3F));
        tmp[2] = char(0x80 | ( code        & 0x3F));
        len = 3;
    } else if (code <= 0x10FFFF) {
        tmp[0] = char(0xF0 | ((code >> 18) & 0xFF));
        tmp[1] = char(0x80 | ((code >> 12) & 0x3F));
        tmp[2] = char(0x80 | ((code >>  6) & 0x3F));
        tmp[3] = char(0x80 | ( code        & 0x3F));
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
    string_t rlt;

    while (true) {
        switch (*++it) {
        case '\"': {
            mnode.set(std::move(rlt));
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
            case 'u':
            {
                string_t ustr;
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
    mnode.set(array_t());

    if (*it == ']') {
        ++it;
        return true;
    }

    node cnode;
    auto& arr = mnode.get<node_t::array_t>();
    
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
inline bool json::parse_key(key_t& key)
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
            case 'u':
            {
                string_t ustr;
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
    mnode.set(object_t());

    if (*it == '}') {
        ++it;
        return true;
    }

    auto& obj = mnode.get<node_t::object_t>(); 
    node cnode;
    key_t key;

    while (true) {
        parse_ws();
        if (!parse_key(key))
            return false;

        parse_ws();
        if (*it == ':')
            ++it;
        else {
            perr = error_code::miss_separator;
            return false;
        }

        if (!parse_value(cnode))
            return false;

        // mnode.object.insert({ std::move(key), std::move(cnode) });
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
    case node_t::array_t:
        return str_array(mnode);

    case node_t::object_t:
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
inline output_t json::str_string(output_t const& src)
{
    output_t ret;
    
    for (auto it = src.begin(); it != src.end(); ++it) {
        switch (*it)
        {
        case '\"':
            ret.append("\\\"");
            break;
        case '\\':
            ret.append("\\");
        default:
            ret.append(&*it, 1);
            break;
        }
    }

    return ret;
}

/**
 * the interface of stringing literal node 
 */
inline bool json::str_literal(node& mnode)
{
    switch (mnode.type()) {
    case node_t::null_t:
        string->append("null");
        break;
    case node_t::bool_t:
        string->append(mnode.get<node_t::bool_t>() ? "true" : "false");
        break;
    case node_t::number_t:
    {
        auto conv = std::to_string(mnode.get<node_t::number_t>());
        string->append(string_t(conv.begin(), conv.end()));
        break;
    }
    case node_t::string_t:
        string->append("\"")
            .append(str_string(mnode.get<node_t::string_t>()))
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
    auto& arr = mnode.get<node_t::array_t>();
    bool sts = false;
    for (auto it = arr.begin(); it != arr.end();) {
        switch (it->type()) {
        case node_t::array_t:
            sts = str_array(*it);
            break;
        case node_t::object_t:
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
    auto& map = mnode.get<node_t::object_t>();
    bool sts = false;
    for (auto it = map.begin(); it != map.end();) {
        string->append("\"")
            .append(str_string(it->first))
            .append("\": ");
        switch (it->second.type()) {
        case node_t::array_t:
            sts = str_array(it->second);
            break;
        case node_t::object_t:
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