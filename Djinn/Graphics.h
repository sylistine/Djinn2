#pragma once

#include <wrl.h>
#include <memory>
#include <vector>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXMath.h>

#include "Logger.h"

namespace Djinn
{
    class Graphics
    {
    public:
        Graphics();
        ~Graphics();
        void Initialize();
        void Update();
    private:
        Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        DXGI_FORMAT backBufferFormat;
        // Initialization.
        void InitDebugLayer();
        void CreateDxgiFactory();
        void CreateDevice();
        // Debug Logging.
        void LogAdapters();
        void LogAdapterOutputs(IDXGIAdapter& adapter);
        void LogOutputDisplayModes(IDXGIOutput& output);
    };


    class GraphicsException : public std::exception
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
}
