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
        void Init(Microsoft::WRL::ComPtr<ID3D12Device> device);
        void Create(UINT64 width, UINT64 height);
        void Reset() { m_texture.Reset(); }
        ID3D12Resource* Resource() { return m_texture.Get(); }
    private:
        bool m_initialized;
        Microsoft::WRL::ComPtr<ID3D12Device> m_device;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_viewHeap;
        UINT m_viewSize;

        DXGI_FORMAT m_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
    };
}
