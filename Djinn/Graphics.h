#pragma once

#include <wrl.h>
#include <memory>
#include <vector>
#include <array>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXMath.h>

#include "Logger.h"

namespace Djinn
{   
    typedef struct OutputInfo {
        DXGI_OUTPUT_DESC desc;
        std::vector<DXGI_MODE_DESC> modeDescs;
    } OutputInfo;

    typedef struct AdapterInfo {
        DXGI_ADAPTER_DESC desc;
        std::vector<OutputInfo> outputs;
    } AdapterInfo;

    class Graphics
    {
    public:
        Graphics(HWND outputWindow);
        ~Graphics();
        void Initialize();
        void Update();
    private:
        Graphics(const Graphics& other) {}
        HWND outputWindow;
        Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

        std::vector<AdapterInfo> adapterInfo;
        DXGI_MODE_DESC preferredOutputMode;

        DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT msaaQualityLevels = 0;
        UINT swapChainBufferCount = 2;

        // Initialization.
        void InitDebugLayer();
        void CreateDxgiFactory();
        void CreateDevice();
        void UpdateAdapterInfo();
        void CreateFence();
        void CheckMSAASupport();
        void CreateCommandObjects();
        void CreateSwapChain();
        void CreateDescriptorHeaps();

        void LogAdapters();
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
