#pragma once
#include <cstdlib>
#include <string>
#include <cassert>
#include <sstream>
#include <vector>
#include <unordered_map>

enum class json_type {
    null_,
    true_,
    false_,
    number_,
    string_,
    array_,
    object_
};

enum class parse_code {
    ok_,
    expect_value_,
    invalid_value_,
    root_singular_,

    invalid_key_,
    miss_split_
};


struct json_node
{
    struct data_t
    {
        std::string string;
        std::vector<json_node> array;
        std::unordered_map<std::string, json_node> member;
        double number;
    } data;
    
    json_type type;
    
    char const* type_name()
    {
        switch (type)
        {
            case json_type::null_:
                return "null";
            case json_type::true_:
                return "true";
            case json_type::false_:
                return "false";
            case json_type::string_:
                return "string";
            case json_type::array_:
                return "array";
            case json_type::object_:
                return "object";
            default:
                return "invalid type";
        }
    }
};


struct json_context
{
    char const* json;
};



parse_code json_parse(json_node&, char const*);
json_type  get_type(json_node&);
parse_code parse_value(json_node&, json_context&);
void       parse_ws(json_context&);
parse_code parse_null(json_node&, json_context&);
parse_code parse_true(json_node&, json_context&);
parse_code parse_false(json_node&, json_context&);
parse_code parse_number(json_node&, json_context&);
double     get_number(json_node&);
parse_code parse_string(json_node&, json_context&);
std::string& get_string(json_node&);
parse_code parse_array(json_node&, json_context&);
std::vector<json_node>& get_array(json_node&);
parse_code parse_object(json_node&, json_context&);


// main api
parse_code json_parse(json_node& node, char const* src)
{
    json_context context;
    node.type = json_type::null_;
    context.json = src;
    return parse_value(node, context);
}


// get node type
json_type get_type(json_node& node)
{
    return node.type;
}

// parse_value
parse_code parse_value(json_node& node, json_context& context)
{
    parse_ws(context);
    switch (*context.json)
    {
    case 'n':
        return parse_null(node, context);

    case 't':
        return parse_true(node, context);

    case 'f':
        return parse_false(node, context);

    case '\"':
        return parse_string(node, context);

    case '[':
        return parse_array(node, context);

    case '{':
        return parse_object(node, context);

    default:
        return parse_number(node, context);

    case '\0':
        return parse_code::expect_value_;
    }
}

// parse_whitespace
void parse_ws(json_context& context)
{
    auto ptr = context.json;
    while (*ptr == ' ' || *ptr == '\n' || *ptr == '\t' || *ptr == '\r')
        ptr++;
    context.json = ptr;
}

// parse_null
parse_code parse_null(json_node& node, json_context& context)
{
    auto ptr = context.json;
    if (ptr[0] != 'n' || ptr[1] != 'u' || ptr[2] != 'l' && ptr[3] != 'l')
        return parse_code::invalid_value_;
    ptr += 4;
    node.type = json_type::null_;
    context.json = ptr;
    return parse_code::ok_;
}

// parse_true
parse_code parse_true(json_node& node, json_context& context)
{
    auto ptr = context.json;
    if (ptr[0] != 't' || ptr[1] != 'r' || ptr[2] != 'u' && ptr[3] != 'e')
        return parse_code::invalid_value_;
    ptr += 4;
    node.type = json_type::true_;
    context.json = ptr;
    return parse_code::ok_;
}

// parse_false
parse_code parse_false(json_node& node, json_context& context)
{
    auto ptr = context.json;
    if (ptr[0] != 'f' || ptr[1] != 'a' || ptr[2] != 'l' && ptr[3] != 's' || ptr[4] != 'e')
        return parse_code::invalid_value_;
    ptr += 5;
    node.type = json_type::false_;
    context.json = ptr;
    return parse_code::ok_;
}

// parse_number
parse_code parse_number(json_node& node, json_context& context)
{
    char* end;
    node.data.number = std::strtod(context.json, &end);
    if (end == context.json)
        return parse_code::invalid_value_;
    context.json = end;
    node.type = json_type::number_;
    return parse_code::ok_;
}

// get_number
double get_number(json_node& node)
{
    assert(node.type == json_type::number_);
    return node.data.number;
}

// parse_string
parse_code parse_string(json_node& node, json_context& context)
{
    auto ptr = context.json;
    std::stringstream stream;

    while (true) {
        switch (*++ptr)
        {
        case '\"':
        {
            node.type = json_type::string_;
            node.data.string = std::move(stream.str());
            context.json = ++ptr;
            return parse_code::ok_;
        }
        case '\0':
            return parse_code::invalid_value_;

        case '\\':
            switch (*++ptr)
            {
            case '\"': stream << '\"'; break;
            case '\\': stream << '\\'; break;
            case '/':  stream << '/'; break;
            case 'b':  stream << '\b'; break;
            case 'f':  stream << '\f'; break;
            case 'n':  stream << '\n'; break;
            case 'r':  stream << '\r'; break;
            case 't':  stream << '\t'; break;
            case 'u':
                /* TODO: handle UNICODE string */
                break;
            default:
                return parse_code::invalid_value_;
            }
            break;
        default:
            stream << *ptr;
            break;
        }
    }
}

// get_string
std::string& get_string(json_node& node)
{
    assert(node.type == json_type::string_);
    return node.data.string;
}

// parse_array
parse_code parse_array(json_node& node, json_context& context)
{
    context.json++;
    parse_ws(context);

    if (*context.json == ']') {
        context.json++;
        node.type == json_type::array_;
        return parse_code::ok_;
    }
    

    while (true) {
        json_node cnode;

        parse_code sts = parse_value(cnode, context);
        if (sts != parse_code::ok_)
            return sts;
        
        node.data.array.push_back(std::move(cnode));

        if (*context.json == ']') {
            context.json++;
            node.type = json_type::array_;
            return parse_code::ok_;
        }

        if (*context.json == ',')
            context.json++;
    }
}

// parse_key
parse_code parse_key(std::string& key, json_context& context)
{
    auto ptr = context.json;
    std::stringstream stream;

    while (true) {
        switch (*++ptr)
        {
        case '\"':
        {
            key = std::move(stream.str());
            context.json = ++ptr;
            return parse_code::ok_;
        }
        case '\0':
            return parse_code::invalid_key_;

        case '\\':
            switch (*++ptr)
            {
            case '\"': stream << '\"'; break;
            case '\\': stream << '\\'; break;
            case '/':  stream << '/'; break;
            case 'b':  stream << '\b'; break;
            case 'f':  stream << '\f'; break;
            case 'n':  stream << '\n'; break;
            case 'r':  stream << '\r'; break;
            case 't':  stream << '\t'; break;
            case 'u':
                /* TODO: handle UNICODE string */
                break;
            default:
                return parse_code::invalid_key_;
            }
            break;
        default:
            stream << *ptr;
            break;
        }
    }
}

// parse_object
parse_code parse_object(json_node& node, json_context& context)
{
    context.json++;
    parse_ws(context);
    if (*context.json == '}') {
        node.type = json_type::object_;
        context.json++;
        return parse_code::ok_;
    }

    while (true) {
        std::string key;
        parse_ws(context);
        parse_code ret = parse_key(key, context);
        if (ret != parse_code::ok_)
            return ret;

        parse_ws(context);
        if (*context.json == ':')
            context.json++;
        else
            return parse_code::miss_split_;
        
        json_node cnode;
        ret = parse_value(cnode, context);
        if (ret != parse_code::ok_)
            return ret;

        node.data.member.insert({std::move(key), std::move(cnode)});
        parse_ws(context);
        
        if (*context.json == '}') {
            node.type = json_type::object_;
            context.json++;
            return parse_code::ok_;
        }

        if (*context.json == ',')
            context.json++;
    }
}