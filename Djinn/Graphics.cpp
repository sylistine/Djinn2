#include "Graphics.h"


using namespace Microsoft::WRL;
using namespace Djinn;


Graphics::Graphics(HWND outputWindow) : outputWindow(outputWindow)
{}


Graphics::~Graphics()
{}


void Graphics::Initialize()
{
    Logger::Log(L"Graphics: DirectX12");

#if _DEBUG
    InitDebugLayer();
#endif

    CreateDxgiFactory();
    CreateDevice();

    

#if _DEBUG
    LogAdapters();
#endif

    CreateFence();
    CheckMSAASupport();
    CreateCommandObjects();
}


void Graphics::Update()
{

}


inline void Graphics::InitDebugLayer()
{
    ComPtr<ID3D12Debug> debugController;
    auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    if (FAILED(result)) throw GraphicsException("Failed to initialize debug layer");
    debugController->EnableDebugLayer();
}


inline void Graphics::CreateDxgiFactory()
{
    auto result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(result)) throw GraphicsException("Failed to create DXGIFactory");
}


inline void Graphics::CreateDevice()
{
    auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(result))
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        result = dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
        if (FAILED(result)) throw GraphicsException("Failed to create default device and warp adapter.");

        result = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if (FAILED(result)) throw GraphicsException("Unable to create any device.");
    }
}


inline void Graphics::CreateFence()
{
    auto result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(result)) throw GraphicsException("Unable to create fence");
}


inline void Graphics::CheckMSAASupport()
{
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = backBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    auto result = device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels, sizeof msQualityLevels);
    msaaQualityLevels = msQualityLevels.NumQualityLevels - 1;

    Logger::Log(std::to_wstring(msaaQualityLevels));
}


inline void Graphics::CreateCommandObjects()
{
    auto commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = commandListType;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    auto result = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(result)) throw GraphicsException("Unable to create command queue.");

    result = device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator));
    if (FAILED(result)) throw GraphicsException("Unable to create command allocator.");

    result = device->CreateCommandList(0, commandListType, commandAllocator.Get(), nullptr,
        IID_PPV_ARGS(&commandList));
    if (FAILED(result)) throw GraphicsException("Unable to create command list.");

    commandList->Close();
}


inline void Graphics::CreateSwapChain()
{
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 4;
    sampleDesc.Quality = msaaQualityLevels;

    DXGI_MODE_DESC modeDesc;
    modeDesc.Width;
    modeDesc.Height;
    modeDesc.RefreshRate.Numerator;
    modeDesc.RefreshRate.Denominator;
    modeDesc.Format = backBufferFormat;
    modeDesc.ScanlineOrdering;
    modeDesc.Scaling;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc = modeDesc;
    swapChainDesc.SampleDesc = sampleDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = swapChainBufferCount;
    swapChainDesc.OutputWindow = outputWindow;
    swapChainDesc.Windowed = false;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    swapChainDesc.SampleDesc = sampleDesc;

    dxgiFactory->CreateSwapChain(device.Get(), &swapChainDesc, swapChain.GetAddressOf());
}


inline void Graphics::CreateDescriptorHeaps()
{

}


void Graphics::GetDisplayModes()
{
}


void Graphics::LogAdapters()
{
    UINT i = 0;
    IDXGIAdapter *current_adapter = nullptr;
    while(dxgiFactory->EnumAdapters(i++, &current_adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        current_adapter->GetDesc(&desc);
        
        std::wstring text = L"Adapter: ";
        text += desc.Description;
        Logger::Log(text);

        LogAdapterOutputs(*current_adapter);
    }
}


void Graphics::LogAdapterOutputs(IDXGIAdapter& adapter)
{
    UINT i = 0;
    IDXGIOutput* current_output = nullptr;
    while (adapter.EnumOutputs(i++, &current_output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        current_output->GetDesc(&desc);

        std::wstring text = L"Output: ";
        text += desc.DeviceName;

        Logger::Log(text);

        LogOutputDisplayModes(*current_output);

        current_output->Release();
        current_output = nullptr;
    }
}


void Graphics::LogOutputDisplayModes(IDXGIOutput& output)
{
    UINT flags = 0;
    UINT count = 0;
    output.GetDisplayModeList(backBufferFormat, flags, &count, nullptr);
    auto modeList = new DXGI_MODE_DESC[count];
    output.GetDisplayModeList(backBufferFormat, flags, &count, modeList);

    std::wstring text = L"";
    for (UINT i = 0; i < count; ++i)
    {
        UINT num = modeList[i].RefreshRate.Numerator;
        UINT den = modeList[i].RefreshRate.Denominator;
        text.append(std::to_wstring(modeList[i].Width) + L"x");
        text.append(std::to_wstring(modeList[i].Height) + L" @ ");
        text.append(std::to_wstring(num) + L"/" + std::to_wstring(den) + L"hz");
        text.append(Logger::WNewLine());
    }
    Logger::Log(text);

    delete[] modeList;
}
