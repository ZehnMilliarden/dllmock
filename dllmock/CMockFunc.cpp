#include "CMockFunc.h"

#include<regex>

CMockFunc::CMockFunc()
{

}

CMockFunc::CMockFunc(const char* szName)
{
    Parse(szName);
}

CMockFunc::~CMockFunc()
{

}

bool CMockFunc::Parse(const char* szName)
{
    if (!szName)
    {
        return false;
    }

    m_str_origin_name = szName;

    bool bRet = false;

    do 
    {
        if (parse_to_c_stdcall())
        {
            break;
        }

        if (parse_to_c_cdecl())
        {
            break;
        }

        if (parse_to_c_fastcall())
        {
            break;
        }

        if (parse_to_cpp_stdcall())
        {
            break;
        }

        if (parse_to_cpp_cdecl())
        {
            break;
        }

        if (parse_to_cpp_fastcall())
        {
            break;
        }

        parse_to_default_call();

    } while (false);

    return false;
}

const char* CMockFunc::GetFuncSign()
{
    return m_str_func_sign.c_str();
}

const char* CMockFunc::GetFuncName()
{
    return m_str_func_name.c_str();
}

const char* CMockFunc::GetFuncOriginName()
{
    return m_str_origin_name.c_str();
}

const char* CMockFunc::GetFuncRetType()
{
    return "void";
}

const char* CMockFunc::GetFuncRetDemo()
{
    return "";
}

bool CMockFunc::parse_to_c_stdcall()
{
    std::string str_pattern("^_([a-zA-Z0-9_]+)@{0,1}([0-9]+)$");
    std::regex r(str_pattern);
    std::smatch results;
    if (!std::regex_match(m_str_origin_name, results, r))
    {
        return false;
    }

    m_str_func_name = results[1].str();
    m_param_count = std::stoi(results[2].str()) / 4;
    
    m_str_func_sign = std::string("extern \"C\" __declspec(dllexport) void __stdcall ")
        .append(m_str_func_name)
        .append("(");

    for (int i = 0; i < m_param_count; ++i)
    {
        m_str_func_sign.append("void* p").append(std::to_string(i)).append("=nullptr");
        if (i < m_param_count - 1)
        {
            m_str_func_sign.append(", ");
        }
    }

    m_str_func_sign.append(")");
    return true;
}

bool CMockFunc::parse_to_c_cdecl()
{
    std::string str_pattern("^_([a-zA-Z0-9_]+)$");
    std::regex r(str_pattern);
    std::smatch results;
    if (!std::regex_match(m_str_origin_name, results, r))
    {
        return false;
    }

    m_str_func_name = results[1].str();

    m_str_func_sign = std::string("extern \"C\" __declspec(dllexport) void __cdecl ")
        .append(m_str_func_name)
        .append("()");
    return true;
}

bool CMockFunc::parse_to_c_fastcall()
{
    std::string str_pattern("^@([a-zA-Z0-9_]+)@{0,1}([0-9]+)$");
    std::regex r(str_pattern);
    std::smatch results;
    if (!std::regex_match(m_str_origin_name, results, r))
    {
        return false;
    }

    m_str_func_name = results[1].str();
    m_param_count = std::stoi(results[2].str()) / 4;

    m_str_func_sign = std::string("extern \"C\" __declspec(dllexport) void __fastcall ")
        .append(m_str_func_name)
        .append("(");

    for (int i = 0; i < m_param_count; ++i)
    {
        m_str_func_sign.append("void* p").append(std::to_string(i)).append("=nullptr");
        if (i < m_param_count - 1)
        {
            m_str_func_sign.append(", ");
        }
    }

    m_str_func_sign.append(")");
    return true;
}

bool CMockFunc::parse_to_cpp_stdcall()
{
    std::regex r(R"(^\?([a-zA-Z0-9_]+)([a-zA-Z0-9_@]*)@@YG([a-zA-Z0]*)(@{0,1})Z$)");
    std::smatch results;
    if (!std::regex_match(m_str_origin_name, results, r))
    {
        return false;
    }

    m_str_func_name = results[1].str();

    m_str_func_sign = std::string("__declspec(dllexport) ");
    m_str_func_sign.append(GetFuncRetType());
    m_str_func_sign.append( " __stdcall ");

    std::vector<std::string> vct_namespace;
    split(results[2].str(), "@", vct_namespace);
    for (auto it : vct_namespace)
    {
        m_str_func_sign.append(it).append("::");
    }

    m_str_func_sign.append(m_str_func_name).append("(");

    if (!results[4].str().empty())
    {
        std::vector<std::string> vct_params;
        if (parse_cpp_call_param(results[3].str(), vct_params))
        {
            for (int i = 0; i < vct_params.size(); ++i)
            {
                if (i != 0)
                {
                    m_str_func_sign.append(", ");
                }
                m_str_func_sign.append(vct_params[i]);
            }
        }
    }

    m_str_func_sign.append(")");

    return true;
}

bool CMockFunc::parse_to_cpp_cdecl()
{
    std::regex r(R"(^\?([a-zA-Z0-9_]+)([a-zA-Z0-9_@]*)@@YA([a-zA-Z0]*)(@{0,1})Z$)");
    std::smatch results;
    if (!std::regex_match(m_str_origin_name, results, r))
    {
        return false;
    }

    m_str_func_name = results[1].str();

    m_str_func_sign = std::string("__declspec(dllexport) ");
    m_str_func_sign.append(GetFuncRetType());
    m_str_func_sign.append(" __cdecl ");

    std::vector<std::string> vct_namespace;
    split(results[2].str(), "@", vct_namespace);
    for (auto it : vct_namespace)
    {
        m_str_func_sign.append(it).append("::");
    }

    m_str_func_sign.append(m_str_func_name).append("(");

    if (!results[4].str().empty())
    {
        std::vector<std::string> vct_params;
        if (parse_cpp_call_param(results[3].str(), vct_params))
        {
            for (int i = 0; i < vct_params.size(); ++i)
            {
                if (i != 0)
                {
                    m_str_func_sign.append(", ");
                }
                m_str_func_sign.append(vct_params[i]);
            }
        }
    }

    m_str_func_sign.append(")");

    return true;
}

bool CMockFunc::parse_to_cpp_fastcall()
{
    std::regex r(R"(\?([a-zA-Z0-9_]+)([a-zA-Z0-9_@]*)@@YI([a-zA-Z0]*)(@{0,1})Z$)");
    std::smatch results;
    if (!std::regex_match(m_str_origin_name, results, r))
    {
        return false;
    }

    m_str_func_name = results[1].str();

    m_str_func_sign = std::string("__declspec(dllexport) ");
    m_str_func_sign.append(GetFuncRetType());
    m_str_func_sign.append(" __fastcall ");

    std::vector<std::string> vct_namespace;
    split(results[2].str(), "@", vct_namespace);
    for (auto it : vct_namespace)
    {
        m_str_func_sign.append(it).append("::");
    }

    m_str_func_sign.append(m_str_func_name).append("(");

    if (!results[4].str().empty())
    {
        std::vector<std::string> vct_params;
        if (parse_cpp_call_param(results[3].str(), vct_params))
        {
            for (int i = 0; i < vct_params.size(); ++i)
            {
                if (i != 0)
                {
                    m_str_func_sign.append(", ");
                }
                m_str_func_sign.append(vct_params[i]);
            }
        }
    }

    m_str_func_sign.append(")");

    return true;
}

bool CMockFunc::parse_to_default_call()
{
    m_str_func_name = m_str_origin_name;
    m_str_func_sign = std::string("extern \"C\" __declspec(dllexport) void ").append(m_str_func_name).append("()");
    return true;
}

bool CMockFunc::parse_cpp_call_param(std::string params, std::vector<std::string>& param_detail)
{
    return true;
}

void CMockFunc::split(std::string str, std::string delimiter, std::vector<std::string>& ret)
{
    if (str.length() <= 0 || delimiter.length() <= 0)
    {
        return;
    }

    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        if (token.length() > 0)
            ret.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    ret.push_back(str);

    std::reverse(ret.begin(), ret.end());
}
