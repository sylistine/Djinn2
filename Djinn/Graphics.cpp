#include "Graphics.h"


using namespace Microsoft::WRL;
using namespace Djinn;


Graphics::Graphics()
{}


Graphics::~Graphics()
{}


void Graphics::Initialize()
{
#if _DEBUG
    InitDebugLayer();
#endif

    CreateDxgiFactory();
    CreateDevice();

#if _DEBUG
    LogAdapters();
#endif
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


void Graphics::LogAdapters()
{
    UINT i = 0;
    IDXGIAdapter *current_adapter = nullptr;
    std::vector<IDXGIAdapter*> adapters;
    while(dxgiFactory->EnumAdapters(i++, &current_adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        current_adapter->GetDesc(&desc);
        
        std::wstring text = L"Adapter: ";
        text += desc.Description;
        text += L"\n";
        Logger::Log(text.c_str());

        adapters.push_back(current_adapter);
    }

    for (auto& adapter : adapters)
    {
        LogAdapterOutputs(*adapter);
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
        text += L"\n";

        Logger::Log(text.c_str());

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

    for (UINT i = 0; i < count; ++i)
    {
        UINT num = modeList[i].RefreshRate.Numerator;
        UINT den = modeList[i].RefreshRate.Denominator;
        std::wstring text =
            std::to_wstring(modeList[i].Width) + L"x" +
            std::to_wstring(modeList[i].Height) + L"@" +
            std::to_wstring(num) + L"/" + std::to_wstring(den) + L"hz\n";

        Logger::Log(text.c_str());
    }
}
