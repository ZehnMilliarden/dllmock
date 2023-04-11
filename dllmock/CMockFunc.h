#pragma once

#include <string>
#include <vector>

class CMockFunc
{
public:
    CMockFunc();
    explicit CMockFunc(const char* szName);
    ~CMockFunc();

public:
    bool Parse(const char* szName);
    const char* GetFuncSign();
    const char* GetFuncName();
    const char* GetFuncOriginName();
    const char* GetFuncRetDemo();
    const char* GetFuncRetType();

private:
    bool parse_to_c_stdcall();
    bool parse_to_c_cdecl();
    bool parse_to_c_fastcall();
    bool parse_to_cpp_stdcall();
    bool parse_to_cpp_cdecl();
    bool parse_to_cpp_fastcall();
    bool parse_to_default_call();
    bool parse_cpp_call_param(std::string params, std::vector<std::string>& param_detail);
    
private:
    void split(std::string str, std::string delimiter, std::vector<std::string>& ret);

private:
    std::string m_str_origin_name;
    std::string m_str_func_sign;
    std::string m_str_func_name;
    int m_param_count = 0;
};