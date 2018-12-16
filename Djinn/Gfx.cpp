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
    m_outputWindow = wnd;
    this->m_width = width;
    this->m_height = height;

    UpdateAdapterInfo();
#if _DEBUG
    LogAdapters();
#endif

    CreateFence();
    CheckMSAASupport();
    CreateCommandObjects();

    CreateSwapChain();

    CreateDescriptorHeaps();

    m_depthStencilTexture.Init(m_device);

    OnResize();
}


void Gfx::Update()
{
    // Reset command list junk.
    auto result = m_commandAllocator->Reset();
    if (FAILED(result)) throw GfxException(result, "Failed to reset command allocator.");
    result = m_commandList->Reset(m_commandAllocator.Get(), nullptr);
    if (FAILED(result)) throw GfxException(result, "Failed to reset command list.");

    // Build command list.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapChainBuffer[m_currentBackBuffer].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    ));
    D3D12_CPU_DESCRIPTOR_HANDLE viewHandle;
    viewHandle.ptr = m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
        m_currentBackBuffer * m_rtvDescSize;
    float clearColor[4] = { 0, 0, 0, 0 };
    m_commandList->ClearRenderTargetView(viewHandle, clearColor, 0, nullptr);
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapChainBuffer[m_currentBackBuffer].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    ));
    m_commandList->Close();
    
    // Submit command list.
    ID3D12CommandList **cmdListArray = new ID3D12CommandList*[1];
    cmdListArray[0] = m_commandList.Get();
    m_commandQueue->ExecuteCommandLists(1, cmdListArray);
    result = m_swapChain->Present(0, 0);
    if (FAILED(result)) throw GfxException(result, "Failed to present swap chain.");
    m_currentBackBuffer = (m_currentBackBuffer + 1) % s_swapChainBufferCount;
    FlushCommandQueue();
    delete[] cmdListArray;
}


void Gfx::GetWindowSize(int& width, int& height)
{
    width = m_preferredOutputMode.Width;
    height = m_preferredOutputMode.Height;
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
    auto result = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));
    if (FAILED(result)) throw GfxException(result, "Failed to create DXGIFactory");
}


inline void Gfx::CreateDevice()
{
    auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
    if (FAILED(result))
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        result = m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
        if (FAILED(result)) throw GfxException(result, "Failed to create default device and warp adapter.");

        result = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        if (FAILED(result)) throw GfxException(result, "Unable to create any device.");
    }
}


void Gfx::UpdateAdapterInfo()
{
    UINT i = 0;
    IDXGIAdapter *current_adapter = nullptr;
    while (m_dxgiFactory->EnumAdapters(i++, &current_adapter) != DXGI_ERROR_NOT_FOUND)
    {
        AdapterInfo newAdapter;
        current_adapter->GetDesc(&newAdapter.desc);

        UINT j = 0;
        IDXGIOutput* current_output = nullptr;
        while (current_adapter->EnumOutputs(j++, &current_output) != DXGI_ERROR_NOT_FOUND)
        {
            UINT modeCount = 0;
            current_output->GetDisplayModeList(m_backBufferFormat, 0, &modeCount, nullptr);
            auto modeList = new DXGI_MODE_DESC[modeCount];
            current_output->GetDisplayModeList(m_backBufferFormat, 0, &modeCount, modeList);

            OutputInfo newOutput;
            current_output->GetDesc(&newOutput.desc);
            for (UINT k = 0U; k < modeCount; ++k)
            {
                if (modeList[k].Width == 1920U && modeList[k].Height == 1080U) {
                    if (m_preferredOutputMode.RefreshRate.Denominator == 0) {
                        m_preferredOutputMode = modeList[k];
                    }
                    else {
                        auto& newRefresh = modeList[k].RefreshRate;
                        double newRate = newRefresh.Numerator / newRefresh.Denominator;
                        auto& oldRefresh = m_preferredOutputMode.RefreshRate;
                        double oldRate = oldRefresh.Numerator / oldRefresh.Denominator;
                        if (newRate > oldRate) {
                            m_preferredOutputMode = modeList[k];
                        }
                    }
                }
                newOutput.modeDescs.push_back(modeList[k]);
            }
            newAdapter.outputs.push_back(newOutput);
            current_output->Release();
            current_output = nullptr;
        }
        m_adapterInfo.push_back(newAdapter);
        current_adapter->Release();
        current_adapter = nullptr;
    }

    if (m_preferredOutputMode.RefreshRate.Numerator == 0) {
        throw GfxException(0, "No supported output modes detected.");
    } else {
        Logger::Write(wstring(L"Preferred output mode found. ")
            + to_wstring(m_preferredOutputMode.Width) + L"x"
            + to_wstring(m_preferredOutputMode.Height));
    }
}


inline void Gfx::CreateFence()
{
    auto result = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(result)) throw GfxException(result, "Unable to create fence");
}


inline void Gfx::CheckMSAASupport()
{
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = m_backBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    auto result = m_device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels, sizeof msQualityLevels);
    m_msaaQualityLevels = msQualityLevels.NumQualityLevels - 1;

    Logger::Write(L"4x MSAA quality levels: " + to_wstring(m_msaaQualityLevels));
}


inline void Gfx::CreateCommandObjects()
{
    auto commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = commandListType;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    auto result = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(result)) throw GfxException(result, "Unable to create command queue.");

    result = m_device->CreateCommandAllocator(
        commandListType,
        IID_PPV_ARGS(m_commandAllocator.GetAddressOf()));
    if (FAILED(result)) throw GfxException(result, "Unable to create command allocator.");

    result = m_device->CreateCommandList(
        0,
        commandListType,
        m_commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(m_commandList.GetAddressOf()));
    if (FAILED(result)) throw GfxException(result, "Unable to create command list.");

    m_commandList->Close();
}


inline void Gfx::CreateSwapChain()
{
    m_swapChain.Reset();

    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality= 0;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_width;
    swapChainDesc.Format = m_backBufferFormat;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc = sampleDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = s_swapChainBufferCount;
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

    auto result = m_dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),
        m_outputWindow,
        &swapChainDesc,
        nullptr,
        nullptr,
        m_swapChain.GetAddressOf());

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
    m_rtvDescSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = s_swapChainBufferCount;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    m_device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS(m_rtvHeap.GetAddressOf()));

    m_cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}


void Gfx::OnResize()
{
    FlushCommandQueue();

    // Command objects reset.
    auto result = m_commandList->Reset(m_commandAllocator.Get(), nullptr);
    if (FAILED(result)) throw GfxException(result, "Unable to reset command list.");

    // Final render resource reset.
    for (auto& buffer : m_swapChainBuffer) {
        buffer.Reset();
    }
    m_depthStencilTexture.Reset();

    result = m_swapChain->ResizeBuffers(
        s_swapChainBufferCount,
        m_width, m_height,
        m_backBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
    if (FAILED(result)) throw GfxException(result, "Unable to resize swap chain buffers.");

    m_currentBackBuffer = 0;

    auto rtvDescHeapHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (auto i = 0U; i < s_swapChainBufferCount; ++i) {
        result = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i]));
        if (FAILED(result))
            throw GfxException(result, "Unable to retrieve buffer from swap chain.");
    
        m_device->CreateRenderTargetView(
            m_swapChainBuffer[i].Get(), nullptr, rtvDescHeapHandle);
    
        rtvDescHeapHandle.ptr += m_rtvDescSize;
    }

    m_depthStencilTexture.Create(m_width, m_height);

    m_commandList->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_depthStencilTexture.Resource(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_DEPTH_WRITE));

    result = m_commandList->Close();
    if (FAILED(result)) throw GfxException(result, "Failed to close commandList.");

    ID3D12CommandList *commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);

    FlushCommandQueue();

    // Update viewport and scissor rect.
    m_screenViewport.TopLeftX = 0;
    m_screenViewport.TopLeftY = 0;
    m_screenViewport.Width = static_cast<float>(m_width);
    m_screenViewport.Height = static_cast<float>(m_height);
    m_screenViewport.MinDepth = 0.0f;
    m_screenViewport.MaxDepth = 1.0f;

    m_scissorRect = { 0, 0, m_width, m_height };
}


void Gfx::FlushCommandQueue()
{
    m_currentFence++;

    auto result = m_commandQueue->Signal(m_fence.Get(), m_currentFence);
    if (FAILED(result)) throw GfxException(result, "Failed to signal fence.");

    if (m_fence->GetCompletedValue() < m_currentFence) {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        result = m_fence->SetEventOnCompletion(m_currentFence, eventHandle);
        if (FAILED(result)) throw GfxException(result, "Failed setting completion event for fence blocking.");
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    m_commandLists.clear();
}


void Gfx::LogAdapters()
{
    wstring log = L"";

    for (auto& adapter : m_adapterInfo) {
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
