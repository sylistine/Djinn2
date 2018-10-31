#pragma once


#include <filesystem>


class Logger
{
public:
    static void Log(const char* msg);
private:
    static Logger logger;
    const char* logAddress;
    Logger();
    ~Logger();
    FILE* file;
    const char* newlinechar;
    size_t newlinesize;
    void Write(const char* msg);
};
