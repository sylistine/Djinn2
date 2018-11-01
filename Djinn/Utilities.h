#pragma once

#include <Windows.h>
#include <string>

namespace Djinn
{
    class Utilities
    {
    public:
        static std::wstring ANSI2UNICODE(const char* str)
        {
            const size_t buffer_size = 256;
            wchar_t buffer[buffer_size];
            MultiByteToWideChar(CP_ACP, 0, str, -1, buffer, buffer_size);
            return std::wstring(buffer);
        }
    };
}
