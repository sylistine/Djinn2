#include "Logger.h"


using namespace Djinn;


Logger::Logger()
{
    auto filepath = "log.txt";
    auto err = fopen_s(&file, filepath, "wb");
}


Logger::~Logger()
{
    fclose(file);
}


Logger Logger::instance = Logger();


void Logger::Write(std::wstring msg)
{
    Write(msg.c_str());
}


/// Static Log that calls instance Write.
void Logger::Write(const wchar_t* msg)
{
    instance.Write_Internal(msg);
}


const char* Logger::NewLine()
{
    return instance.newline;
}


const wchar_t* Logger::WNewLine()
{
    return instance.wnewline;
}


void Logger::Write_Internal(const wchar_t* msg)
{
    auto len = 0;
    for (auto i = 0; i < 8192; ++i) {
        if (msg[i] == '\0') break;
        len++;
    }
    fwrite(Utilities::UNICODE2ANSI(msg).c_str(), len, 1, file);
    fwrite(newline, newlinesize, 1, file);
    fwrite(newline, newlinesize, 1, file);
}