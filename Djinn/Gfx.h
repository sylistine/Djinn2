#pragma once

#include <wrl.h>
#include <memory>
#include <vector>
#include <map>
#include <array>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXMath.h>

#include "d3dx12.h"

#include "Logger.h"
#include "GfxException.h"

#include "DepthStencilResource.h"

namespace Djinn::Graphics
{
    typedef struct OutputInfo {
        DXGI_OUTPUT_DESC desc;
        std::vector<DXGI_MODE_DESC> modeDescs;
    } OutputInfo;

    typedef struct AdapterInfo {
        DXGI_ADAPTER_DESC desc;
        std::vector<OutputInfo> outputs;
    } AdapterInfo;

    class Gfx
    {
    public:
        static Gfx& Context() {
            static Gfx instance;
            return instance;
        }
        ~Gfx();
        Gfx(const Gfx&) = delete;
        void operator=(const Gfx&) = delete;
        void Initialize(HWND outputWindow, int width, int height);
        void Update();
        void GetWindowSize(int& width, int& height);
    private:
        Gfx();

        static const UINT swapChainBufferCount = 2;
        HWND outputWindow;
        int width;
        int height;

        // Primary object resources.
        Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
        Microsoft::WRL::ComPtr<ID3D12Device> device;

        // Commands.
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        UINT64 currentFence;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        std::vector<ID3D12CommandList *> commandLists;


        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
        UINT rtvDescSize;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
        UINT dsvDescSize;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvHeap;
        UINT cbvSrvUavDescSize;

        // Resources
        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer[swapChainBufferCount];
        UINT currentBackBuffer;
        DepthStencilResource depthStencilTexture;

        D3D12_VIEWPORT screenViewport;
        D3D12_RECT scissorRect;

        std::vector<AdapterInfo> adapterInfo;
        DXGI_MODE_DESC preferredOutputMode;

        DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT msaaQualityLevels = 0;

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

        void OnResize();

        void FlushCommandQueue();

        void LogAdapters();
    };
}
