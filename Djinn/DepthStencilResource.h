#pragma once

#include <wrl.h>

#include <dxgi1_4.h>
#include <d3d12.h>

#include "GfxException.h"

namespace Djinn::Graphics {
    class DepthStencilResource
    {
    public:
        DepthStencilResource();
        ~DepthStencilResource();
        void Init(
            Microsoft::WRL::ComPtr<ID3D12Device> device,
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
        void Create(UINT64 width, UINT64 height);
        void Reset() { depthStencilTexture.Reset(); }
        ID3D12Resource* Resource() { return depthStencilTexture.Get(); }
    private:
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
        bool initialized;

        DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilTexture;
    };
}
