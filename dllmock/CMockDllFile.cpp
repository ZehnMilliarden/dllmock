#include "CMockDllFile.h"

#include <DbgHelp.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <xiosbase>

#pragma comment(lib, "Dbghelp.lib")


#ifdef _WIN64
typedef DWORD_PTR DWORD_PTR_T;
typedef ULONGLONG ULONGLONG_T;
#else
typedef DWORD DWORD_PTR_T;
typedef DWORD ULONGLONG_T;
#endif

CMockDllFile::CMockDllFile()
{

}

CMockDllFile::CMockDllFile(LPCWSTR lpszDllFile)
    : m_strDllFile(lpszDllFile)
{

}

CMockDllFile::~CMockDllFile()
{

}

void CMockDllFile::DumpFunctions()
{
    if (!GetDLLFileExports())
    {
        std::cout << "Error: " << ::GetLastError();
    }

    DWORD dwIndex = 0;
    for (auto it : m_lstFuncName)
    {
        std::cout <<  "Index #" << dwIndex++ << " : " << it->GetFuncSign() << std::endl;
    }
}

void CMockDllFile::SetDllFile(LPCWSTR lpszDllFile)
{
    if (!lpszDllFile)
        return;

    m_strDllFile = lpszDllFile;
}

bool CMockDllFile::MockDll(bool FillContext)
{
    bool bRet = GetDLLFileExports()
        && MakeCode(FillContext)
        && BuildCode();

    return false;
}

bool CMockDllFile::GetDLLFileExports()
{
    bool bRet = false;

    HANDLE hFile = NULL;
    HANDLE hFileMapping = NULL;
    LPVOID lpFileBase = NULL;
    PIMAGE_DOS_HEADER pImg_DOS_Header = NULL;
    PIMAGE_NT_HEADERS pImg_NT_Header = NULL;
    PIMAGE_EXPORT_DIRECTORY pImg_Export_Dir = NULL;

    do 
    {

        hFile = ::CreateFile(m_strDllFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        if (hFile == INVALID_HANDLE_VALUE)
            break;

        hFileMapping = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hFileMapping == 0)
        {
            break;
        }

        lpFileBase = ::MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
        if (!lpFileBase)
        {
            break;
        }

        pImg_DOS_Header = (PIMAGE_DOS_HEADER)lpFileBase;
        pImg_NT_Header = (PIMAGE_NT_HEADERS)(
            (ULONGLONG_T)pImg_DOS_Header + (ULONGLONG_T)pImg_DOS_Header->e_lfanew);

        if (::IsBadReadPtr(pImg_NT_Header, sizeof(IMAGE_NT_HEADERS))
            || pImg_NT_Header->Signature != IMAGE_NT_SIGNATURE)
        {
            break;
        }

        pImg_Export_Dir = (PIMAGE_EXPORT_DIRECTORY)pImg_NT_Header->OptionalHeader
            .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        if (!pImg_Export_Dir)
        {
            break;
        }

        pImg_Export_Dir = (PIMAGE_EXPORT_DIRECTORY)::ImageRvaToVa(pImg_NT_Header,
            pImg_DOS_Header, (ULONGLONG_T)pImg_Export_Dir, 0);

        DWORD_PTR_T** ppdwNames = (DWORD_PTR_T**)pImg_Export_Dir->AddressOfNames;

        ppdwNames = (PDWORD_PTR*)::ImageRvaToVa(pImg_NT_Header,
            pImg_DOS_Header, (ULONGLONG_T)ppdwNames, 0);
        if (!ppdwNames)
        {
            break;
        }

        m_lstFuncName.clear();

        UINT nNumberOfNames = pImg_Export_Dir->NumberOfNames;

        for (UINT i = 0; i < nNumberOfNames; i++)
        {
            char* szFunc = (PSTR)::ImageRvaToVa(pImg_NT_Header, pImg_DOS_Header, (DWORD)*ppdwNames, 0);
            if (szFunc)
            {
                m_lstFuncName.push_back(std::make_shared<CMockFunc>(szFunc));
            }
            ppdwNames = (PDWORD_PTR *)(((ULONGLONG_T)(PDWORD_PTR*)ppdwNames) + 4);
        }

        bRet = true;

    } while (false);

    if (lpFileBase)
        ::UnmapViewOfFile(lpFileBase);

    if (hFileMapping)
        ::CloseHandle(hFileMapping);

    if (hFile && hFile != INVALID_HANDLE_VALUE)
        ::CloseHandle(hFile);

    return bRet;
}

#ifndef CREATE_CODE_FILE_BEGIN
#define CREATE_CODE_FILE_BEGIN(file) {\
    std::wofstream outfile;\
    outfile.open(file);\
    std::wstringstream str_code_context;\
    {
#endif

#ifndef MAKE_CODE_LINE
#define MAKE_CODE_LINE(codeline) {str_code_context << codeline << std::endl;}
#endif

#ifndef MAKE_CODE_EMPTY_LINE
#define MAKE_CODE_EMPTY_LINE {str_code_context << std::endl;}
#endif

#ifndef CREATE_CODE_FILE_END
#define CREATE_CODE_FILE_END \
    } while(false);\
    outfile << str_code_context.str();\
    outfile.close();\
    }
#endif

bool CMockDllFile::MakeCode(bool FillContext)
{
    std::wstring strCodeFile = m_strDllFile;
    strCodeFile.append(L".cpp");

    CREATE_CODE_FILE_BEGIN(strCodeFile);

    MAKE_CODE_LINE(L"#include <Windows.h>");

    for (auto it : m_lstFuncName)
    {
        MAKE_CODE_EMPTY_LINE;
        char szCodeLine[512] = { 0 };
        sprintf_s(szCodeLine, "%s {", it->GetFuncSign());
        MAKE_CODE_LINE(szCodeLine);
        if (FillContext)
        {
            char szCodeFill[512] = { 0 };
            sprintf_s(szCodeLine, "    ::MessageBoxA(NULL, \"%s\", \"Tip\", NULL);", it->GetFuncOriginName());
            MAKE_CODE_LINE(szCodeLine);
        }
        
        {
            char szCodeLine[512] = { 0 };
            sprintf_s(szCodeLine, "    return %s;", it->GetFuncRetDemo());
            MAKE_CODE_LINE(szCodeLine);
        }
        MAKE_CODE_LINE(L"}");
    }

    MAKE_CODE_EMPTY_LINE;
    MAKE_CODE_LINE(L"BOOL APIENTRY DllMain(HMODULE h,DWORD dw,LPVOID lp)");
    MAKE_CODE_LINE(L"{");
    MAKE_CODE_LINE(L"    switch (dw)");
    MAKE_CODE_LINE(L"    {");
    MAKE_CODE_LINE(L"        case DLL_PROCESS_ATTACH:");
    MAKE_CODE_LINE(L"            ::DisableThreadLibraryCalls(h);");
    MAKE_CODE_LINE(L"            break;");
    MAKE_CODE_LINE(L"        default:");
    MAKE_CODE_LINE(L"            break;");
    MAKE_CODE_LINE(L"    }");
    MAKE_CODE_LINE(L"    return TRUE;");
    MAKE_CODE_LINE(L"}");
    CREATE_CODE_FILE_END;

    return false;
}

bool CMockDllFile::BuildCode()
{
    return false;
}
