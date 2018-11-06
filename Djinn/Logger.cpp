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


/// Static Log that calls instance Write.
void Logger::Log(const wchar_t* msg)
{
    logger.Write(msg);
}


void Logger::Log(std::wstring msg)
{
    Log(msg.c_str());
}

const char* Logger::NewLine()
{
    return logger.newline;
}


const wchar_t* Logger::WNewLine()
{
    return logger.wnewline;
}


void Logger::Write(const wchar_t* msg)
{
    auto len = 0;
    for (auto i = 0; i < 8192; ++i) {
        if (msg[i] == '\0') break;
        len++;
    }
    fwrite(Djinn::Utilities::UNICODE2ANSI(msg).c_str(), len, 1, file);
    fwrite(newline, newlinesize, 1, file);
    fwrite(newline, newlinesize, 1, file);
}