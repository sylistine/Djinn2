#include "DepthStencilResource.h"

using namespace Djinn::Graphics;
using namespace Microsoft::WRL;


DepthStencilResource::DepthStencilResource()
{
}


DepthStencilResource::~DepthStencilResource()
{
}


void DepthStencilResource::Init(
    ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap> heap)
{
    this->device = device;
    this->heap = heap;
    initialized = true;
}


void DepthStencilResource::Create(UINT64 width, UINT64 height)
{
    if (!initialized) throw GfxException(0, "Depth Stencil Texture Not Initialized.");

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = depthStencilFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    auto result = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &clearValue,
        IID_PPV_ARGS(depthStencilTexture.GetAddressOf()));
    if (FAILED(result)) throw GfxException(result, "Unable to create Depth Stencil Resource.");

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = depthStencilFormat;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(
        depthStencilTexture.Get(),
        &dsvDesc,
        heap->GetCPUDescriptorHandleForHeapStart());
}
