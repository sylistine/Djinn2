#include "Logger.h"


Logger::Logger()
{
    auto filepath = "log.txt";
    auto err = fopen_s(&file, filepath, "wb");
}


Logger::~Logger()
{
    fclose(file);
}


Logger Logger::logger = Logger();


void Logger::Log(const wchar_t* msg)
{
    logger.Write(msg);
}


void Logger::Write(const wchar_t* msg)
{
    auto len = 0;
    for (auto i = 0; i < 256; ++i) {
        if (msg[i] == '\0') break;
        len++;
    }
    fwrite(msg, len, 1, file);
    fwrite(newlinechar, newlinesize, 1, file);
}