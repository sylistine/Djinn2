#include "Logger.h"


Logger::Logger()
{
    auto filepath = "log.txt";
    auto err = fopen_s(&file, filepath, "wb");

#ifdef _WIN32
    newlinechar = "\r\n";
    newlinesize = 2;
#else
    newlinechar = "\n";
    newlinesize = 1;
#endif
}


Logger::~Logger()
{
    fclose(file);
}


Logger Logger::logger = Logger();


void Logger::Log(const char* msg)
{
    logger.Write(msg);
}


void Logger::Write(const char* msg)
{
    auto len = 0;
    for (auto i = 0; i < 256; ++i) {
        if (msg[i] == '\0') break;
        len++;
    }
    fwrite(msg, len, 1, file);
    fwrite(newlinechar, newlinesize, 1, file);
}