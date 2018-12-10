#pragma once

#include <Windows.h>

#include <exception>
#include <map>


namespace Djinn::Graphics {
    class GfxException : public std::exception
    {
    public:
        GfxException() : GfxException(0, "") {}
        GfxException(HRESULT hresult, char const* error) {
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
