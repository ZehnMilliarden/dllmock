#pragma once

#include <Windows.h>
#include <string>
#include <list>
#include <memory>

#include "CMockFunc.h"

class CMockDllFile
{
public:
    CMockDllFile();

    CMockDllFile(LPCWSTR lpszDllFile);

    ~CMockDllFile();

public:
    void DumpFunctions();

    void SetDllFile(LPCWSTR lpszDllFile);

    bool MockDll(bool FillContext);

private:
    bool GetDLLFileExports();

    bool MakeCode(bool FillContext);

    bool BuildCode();

private:
    std::list< std::shared_ptr<CMockFunc> > m_lstFuncName;
    std::wstring m_strDllFile;
};

