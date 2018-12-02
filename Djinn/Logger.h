#pragma once


#include <filesystem>

#include "Djinn.h"


namespace Djinn
{
    class Logger
    {
    public:
        static void Write(const wchar_t* msg);
        static void Write(const std::wstring msg);
        static const char* NewLine();
        static const wchar_t* WNewLine();
    private:
        static Logger instance;
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
        void Write_Internal(const wchar_t* msg);
    };
}
