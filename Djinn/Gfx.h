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

        static const UINT s_swapChainBufferCount = 2;
        HWND m_outputWindow;
        int m_width;
        int m_height;

        // Primary object resources.
        Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
        Microsoft::WRL::ComPtr<ID3D12Device> m_device;

        // Commands.
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        UINT64 m_currentFence;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        std::vector<ID3D12CommandList *> m_commandLists;


        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        UINT m_rtvDescSize;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
        UINT m_cbvSrvUavDescSize;

        // Resources
        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainBuffer[s_swapChainBufferCount];
        UINT m_currentBackBuffer;

        DepthStencilResource m_depthStencilTexture;

        D3D12_VIEWPORT m_screenViewport;
        D3D12_RECT m_scissorRect;

        std::vector<AdapterInfo> m_adapterInfo;
        DXGI_MODE_DESC m_preferredOutputMode;

        DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT m_msaaQualityLevels = 0;

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
