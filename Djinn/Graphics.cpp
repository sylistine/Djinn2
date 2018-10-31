#include "Graphics.h"


using namespace Microsoft::WRL;
using namespace Djinn;


Graphics::Graphics()
{}


Graphics::~Graphics()
{}


void Graphics::Initialize()
{
#ifdef _DEBUG
    InitDebugLayer();
#endif
}

void Graphics::Update()
{

}

void Graphics::InitDebugLayer()
{
    ComPtr<ID3D12Debug> debugController;
    auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    if (FAILED(result)) throw GraphicsException("Failed to initialize debug layer");
    debugController->EnableDebugLayer();
}
