#pragma once


#include <filesystem>


class Logger
{
public:
    static void Log(const wchar_t* msg);
private:
    static Logger logger;
    const char* logAddress;
    Logger();
    ~Logger();
    FILE* file;
#ifdef _WIN32
    const char* newlinechar = "\r\n";
    size_t newlinesize = 2;
#else
    const char* newlinechar = "\n";
    size_t newlinesize = 1;
#endif
    void Write(const wchar_t* msg);
};
