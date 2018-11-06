#pragma once


#include <filesystem>

#include "Djinn.h"


class Logger
{
public:
    static void Log(const wchar_t* msg);
    static void Log(const std::wstring msg);
    static const char* NewLine();
    static const wchar_t* WNewLine();
private:
    static Logger logger;
#ifdef _WIN32
    const char* newline = "\r\n";
    const wchar_t* wnewline = L"\r\n";
    size_t newlinesize = 2;
#else
    const char* newline = "\n";
    const wchar_t* wnewline = L"\n";
    size_t newlinesize = 2;
#endif
    const char* logAddress;
    Logger();
    ~Logger();
    FILE* file;
    void Write(const wchar_t* msg);
};
