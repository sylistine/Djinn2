#pragma once

#include <wrl.h>
#include <memory>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXMath.h>

namespace Djinn
{
    class GraphicsException : std::exception
    {
    public:
        GraphicsException() : GraphicsException("Unspecified cause.") {};
        GraphicsException(char const* error) : error(error) {};
        char const* what() const override
        {
            char str[256] = "DirectX12 encountered an exception: ";
            auto err = strcat_s(str, 256, error);
            return str;
        };
    private:
        char const* error;
    };


    class Graphics
    {
    public:
        Graphics();
        ~Graphics();
        void Initialize();
        void Update();
    private:
        void InitDebugLayer();
    };
}
