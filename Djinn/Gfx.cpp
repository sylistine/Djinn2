#include "Gfx.h"


using namespace std;
using namespace Microsoft::WRL;
using namespace Djinn::Graphics;


Gfx::Gfx()
{
    Logger::Write(L"Initializing DirectX12.");

#if _DEBUG
    InitDebugLayer();
#endif

    CreateDxgiFactory();
    CreateDevice();
}


Gfx::~Gfx()
{
}


void Gfx::Initialize(HWND wnd, int width, int height)
{
    outputWindow = wnd;
    this->width = width;
    this->height = height;

    UpdateAdapterInfo();
#if _DEBUG
    LogAdapters();
#endif

    CreateFence();
    CheckMSAASupport();
    CreateCommandObjects();

    CreateSwapChain();

    CreateDescriptorHeaps();

    depthStencilTexture.Init(device, dsvHeap);
    OnResize();
}


void Gfx::Update()
{

}


void Gfx::GetWindowSize(int& width, int& height)
{
    width = preferredOutputMode.Width;
    height = preferredOutputMode.Height;
}


inline void Gfx::InitDebugLayer()
{
    ComPtr<ID3D12Debug> debugController;
    auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    if (FAILED(result)) throw GfxException(result, "Failed to initialize debug layer");
    debugController->EnableDebugLayer();
}


inline void Gfx::CreateDxgiFactory()
{
    Logger::Write(L"Creating dxgi factory.");
    auto result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(result)) throw GfxException(result, "Failed to create DXGIFactory");
}


inline void Gfx::CreateDevice()
{
    Logger::Write(L"Creating device.");
    auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(result))
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        result = dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
        if (FAILED(result)) throw GfxException(result, "Failed to create default device and warp adapter.");

        result = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if (FAILED(result)) throw GfxException(result, "Unable to create any device.");
    }
}


void Gfx::UpdateAdapterInfo()
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
            for (UINT k = 0U; k < modeCount; ++k)
            {
                if (modeList[k].Width == 1920U && modeList[k].Height == 1080U) {
                    if (preferredOutputMode.RefreshRate.Denominator == 0) {
                        preferredOutputMode = modeList[k];
                    }
                    else {
                        auto& newRefresh = modeList[k].RefreshRate;
                        double newRate = newRefresh.Numerator / newRefresh.Denominator;
                        auto& oldRefresh = preferredOutputMode.RefreshRate;
                        double oldRate = oldRefresh.Numerator / oldRefresh.Denominator;
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
        throw GfxException(0, "No supported output modes detected.");
    } else {
        Logger::Write(wstring(L"Preferred output mode found. ")
            + to_wstring(preferredOutputMode.Width) + L"x"
            + to_wstring(preferredOutputMode.Height));
    }
}


inline void Gfx::CreateFence()
{
    auto result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(result)) throw GfxException(result, "Unable to create fence");
}


inline void Gfx::CheckMSAASupport()
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

    Logger::Write(L"4x MSAA quality levels: " + to_wstring(msaaQualityLevels));
}


inline void Gfx::CreateCommandObjects()
{
    auto commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = commandListType;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    auto result = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(result)) throw GfxException(result, "Unable to create command queue.");

    result = device->CreateCommandAllocator(
        commandListType,
        IID_PPV_ARGS(commandAllocator.GetAddressOf()));
    if (FAILED(result)) throw GfxException(result, "Unable to create command allocator.");

    result = device->CreateCommandList(
        0,
        commandListType,
        commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(commandList.GetAddressOf()));
    if (FAILED(result)) throw GfxException(result, "Unable to create command list.");

    commandList->Close();
}


inline void Gfx::CreateSwapChain()
{
    swapChain.Reset();

    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality= 0;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width = width;
    swapChainDesc.Height = width;
    swapChainDesc.Format = backBufferFormat;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc = sampleDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = swapChainBufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
    fullscreenDesc.RefreshRate.Numerator = 60;
    fullscreenDesc.RefreshRate.Denominator = 1;
    fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    fullscreenDesc.Windowed = false;

    auto result = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(),
        outputWindow,
        &swapChainDesc,
        nullptr,
        nullptr,
        swapChain.GetAddressOf());

    if (FAILED(result))
    {
        throw GfxException(result, (
            string("Unable to create swap chain. ") +
            to_string(result)
            ).c_str());
    }
}


inline void Gfx::CreateDescriptorHeaps()
{
    // RTV STUFF
    rtvDescSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = swapChainBufferCount;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS(rtvHeap.GetAddressOf()));

    // DSV STUFF
    dsvDescSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(
        &dsvHeapDesc,
        IID_PPV_ARGS(dsvHeap.GetAddressOf()));

    cbvSrvUavDescSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}


void Gfx::OnResize()
{
    FlushCommandQueue();

    // Command objects reset.
    auto result = commandList->Reset(commandAllocator.Get(), nullptr);
    if (FAILED(result)) throw GfxException(result, "Unable to reset command list.");

    // Final render resource reset.
    for (auto& buffer : swapChainBuffer) {
        buffer.Reset();
    }
    depthStencilTexture.Reset();

    result = swapChain->ResizeBuffers(
        swapChainBufferCount,
        width, height,
        backBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
    if (FAILED(result)) throw GfxException(result, "Unable to resize swap chain buffers.");

    currentBackBuffer = 0;

    auto rtvDescHeapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (auto i = 0U; i < swapChainBufferCount; ++i) {
        result = swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffer[i]));
        if (FAILED(result))
            throw GfxException(result, "Unable to retrieve buffer from swap chain.");
    
        device->CreateRenderTargetView(
            swapChainBuffer[i].Get(), nullptr, rtvDescHeapHandle);
    
        rtvDescHeapHandle.ptr += rtvDescSize;
    }

    depthStencilTexture.Create(width, height);

    commandList->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            depthStencilTexture.Resource(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_DEPTH_WRITE));

    result = commandList->Close();
    if (FAILED(result)) throw GfxException(result, "Failed to close commandList.");

    ID3D12CommandList *commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, commandLists);

    FlushCommandQueue();

    // Update viewport and scissor rect.
    screenViewport.TopLeftX = 0;
    screenViewport.TopLeftY = 0;
    screenViewport.Width = static_cast<float>(width);
    screenViewport.Height = static_cast<float>(height);
    screenViewport.MinDepth = 0.0f;
    screenViewport.MaxDepth = 1.0f;

    scissorRect = { 0, 0, width, height };
}


void Gfx::FlushCommandQueue()
{
    currentFence++;

    auto result = commandQueue->Signal(fence.Get(), currentFence);
    if (FAILED(result)) throw GfxException(result, "Failed to signal fence.");

    if (fence->GetCompletedValue() < currentFence) {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        result = fence->SetEventOnCompletion(currentFence, eventHandle);
        if (FAILED(result)) throw GfxException(result, "Failed setting completion event for fence blocking.");
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    commandLists.clear();
}


void Gfx::LogAdapters()
{
    wstring log = L"";

    for (auto& adapter : adapterInfo) {
        log += wstring(L"Adapter: ") + adapter.desc.Description;
        log += Logger::WNewLine();
        for (auto& output : adapter.outputs) {
            log += wstring(L"Output: ") + output.desc.DeviceName;
            log += Logger::WNewLine();
            /*for (auto& mode : output.modeDescs) {
                log += to_wstring(mode.)
                log += to_wstring(mode.Width);
                log += L"x";
                log += to_wstring(mode.Height);
                log += L" @ ";
                log += to_wstring(mode.RefreshRate.Numerator / mode.RefreshRate.Denominator);
                log += L"hz";
                log += Logger::WNewLine();
            }*/
        }
    }
    Logger::Write(log);
}
