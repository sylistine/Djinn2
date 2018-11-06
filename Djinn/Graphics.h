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
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        DXGI_FORMAT backBufferFormat;
        UINT msaaQualityLevels = 0;
        // Initialization.
        void InitDebugLayer();
        void CreateDxgiFactory();
        void CreateDevice();
        void CreateFence();
        void CheckMSAASupport();
        void CreateCommandObjects();
        void CreateSwapChain();
        void CreateDescriptorHeaps();
        // Debug Logging.
        void LogAdapters();
        void LogAdapterOutputs(IDXGIAdapter& adapter);
        void LogOutputDisplayModes(IDXGIOutput& output);
    };


    class GraphicsException : public std::exception
    {
    public:
        GraphicsException() : GraphicsException("Unspecified cause.") {};
        GraphicsException(char const* error) {
            auto err = strcat_s(msg, 256, error);
        };
        char const* what() const override
        {
            return msg;
        };
    private:
        char msg[256] = "DirectX12 encountered an exception: ";
    };
}
