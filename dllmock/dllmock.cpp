// dllmock.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CMockDllFile.h"
#include "cmdline.h"
#include <atlstr.h>

int main(int argc, char* argv[])
{
    cmdline::parser param;
    param.add<std::string>("dll", 'd', "dll path", true);
    param.add<int>("mock", 'm', "mock dll", false);
    param.add("view", 'v', "view export functions");
    param.add("help", 'h', "print this message");

    bool bOk = param.parse(argc, argv);
    if (!bOk)
    {
        std::cerr << param.error() << std::endl << param.usage();
        return 0;
    }

    if (argc == 1 || param.exist("help")) {
        std::cerr << param.usage();
        return 0;
    }

    if (!param.exist("dll"))
    {
        std::cerr << param.usage();
        return 0;
    }

    BOOL bMock = param.exist("mock");
    BOOL bView = param.exist("view");

    std::string dllfile = param.get<std::string>("dll");
    CMockDllFile mock((LPCWSTR)CA2W(dllfile.c_str()));

    if (bMock)
    {
        mock.MockDll(param.get<int>("mock"));
    }

    mock.DumpFunctions();

    return 0;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
