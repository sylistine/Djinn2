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
        static Graphics& Context(){
            static Graphics instance;
            return instance;
        }
        ~Graphics();
        Graphics(const Graphics&) = delete;
        void operator=(const Graphics&) = delete;
        void Initialize(HWND outputWindow, int width, int height);
        void Update();
        void GetWindowSize(int& width, int& height);
    private:
        Graphics() {};

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
        Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilTexture;

        D3D12_VIEWPORT screenViewport;
        D3D12_RECT scissorRect;

        std::vector<AdapterInfo> adapterInfo;
        DXGI_MODE_DESC preferredOutputMode;

        DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
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

    class GraphicsException : public std::exception
    {
    public:
        GraphicsException() : GraphicsException(0, "") {}
        GraphicsException(HRESULT hresult, char const* error) {
            auto errcode = (D3D12Errors.find(hresult)) != D3D12Errors.end() ? D3D12Errors[hresult] : "Unspecified error.";
            strcat_s(msg, 256, error);
            strcat_s(msg, 256, " Error code: ");
            strcat_s(msg, 256, errcode);
        };
        char const* what() const override {
            return msg;
        };
    private:
        char msg[256] = "DirectX12 encountered an exception: ";
        //HRESULT translation
        std::map<unsigned int, const char*> D3D12Errors = {
            { 0x80070057, "E_INVALIDARG" }
        };
    };
}
