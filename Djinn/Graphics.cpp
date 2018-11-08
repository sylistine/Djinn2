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

    UpdateAdapterInfo();

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


void Graphics::UpdateAdapterInfo()
{
    UINT i = 0;
    IDXGIAdapter *current_adapter = nullptr;
    while (dxgiFactory->EnumAdapters(i++, &current_adapter) != DXGI_ERROR_NOT_FOUND)
    {
        AdapterInfo newAdapter;
        current_adapter->GetDesc(&newAdapter.desc);

        UINT j = 0;
        IDXGIOutput* current_output = nullptr;
        while (current_adapter->EnumOutputs(j++, &current_output) != DXGI_ERROR_NOT_FOUND)
        {
            UINT modeCount = 0;
            current_output->GetDisplayModeList(backBufferFormat, 0, &modeCount, nullptr);
            auto modeList = new DXGI_MODE_DESC[modeCount];
            current_output->GetDisplayModeList(backBufferFormat, 0, &modeCount, modeList);

            OutputInfo newOutput;
            current_output->GetDesc(&newOutput.desc);
            for (auto k = 0; k < modeCount; ++k)
            {
                if (modeList[k].Width == 1920U && modeList[k].Height == 1080U) {
                    if (preferredOutputMode.RefreshRate.Denominator == 0) {
                        preferredOutputMode = modeList[k];
                    }
                    else {
                        auto& newRefresh = modeList[k].RefreshRate;
                        float newRate = newRefresh.Numerator / newRefresh.Denominator;
                        auto& oldRefresh = preferredOutputMode.RefreshRate;
                        float oldRate = oldRefresh.Numerator / oldRefresh.Denominator;
                        if (newRate > oldRate) {
                            preferredOutputMode = modeList[k];
                        }
                    }
                }
                newOutput.modeDescs.push_back(modeList[k]);
            }
            newAdapter.outputs.push_back(newOutput);
            current_output->Release();
            current_output = nullptr;
        }
        adapterInfo.push_back(newAdapter);
        current_adapter->Release();
        current_adapter = nullptr;
    }

    if (preferredOutputMode.RefreshRate.Numerator == 0) {
        throw GraphicsException("No supported output modes detected.");
    } else {
        Logger::Log(std::wstring(L"Preferred output mode found. ")
            + std::to_wstring(preferredOutputMode.Width) + L"x"
            + std::to_wstring(preferredOutputMode.Height));
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

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc = preferredOutputMode;
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


void Graphics::LogAdapters()
{
    std::wstring log = L"";

    for (auto& adapter : adapterInfo) {
        log += adapter.desc.Description;
        log += Logger::WNewLine();
        for (auto& output : adapter.outputs) {
            log += output.desc.DeviceName;
            log += Logger::WNewLine();
            for (auto& mode : output.modeDescs) {
                log += std::to_wstring(mode.Width);
                log += L"x";
                log += std::to_wstring(mode.Height);
                log += L" @ ";
                log += std::to_wstring(mode.RefreshRate.Numerator)
                    + L"/"
                    + std::to_wstring(mode.RefreshRate.Denominator);
                log += Logger::WNewLine();
            }
        }
    }
    Logger::Log(log);
}
