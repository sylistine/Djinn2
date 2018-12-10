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
            auto errcode = (D3D12Errors.find(hresult)) != D3D12Errors.end() ?
                D3D12Errors[hresult] : "Unspecified error.";
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
            { 0x00000000, "S_OK" },
            { 0x80004001, "E_NOTIMPL" },
            { 0x80004002, "E_NOINTERFACE" },
            { 0x80004003, "E_POINTER" },
            { 0x80004004, "E_ABORT" },
            { 0x80004005, "E_FAIL" },
            { 0x8000FFFF, "E_UNEXPECTED" },
            { 0x80070005, "E_ACCESSDENIED" },
            { 0x80070006, "E_HANDLE" },
            { 0x8007000E, "E_OUTOFMEMORY" },
            { 0x80070057, "E_INVALIDARG" },
            { 0x887A0001, "DXGI_ERROR_INVALID_CALL" },
            { 0x887A0002, "DXGI_ERROR_NOT_FOUND" },
            { 0x887A0003, "DXGI_ERROR_MORE_DATA" },
            { 0x887A0004, "DXGI_ERROR_UNSUPPORTED" },
            { 0x887A0005, "DXGI_ERROR_DEVICE_REMOVED" },
            { 0x887A0006, "DXGI_ERROR_DEVICE_HUNG" },
            { 0x887A0007, "DXGI_ERROR_DEVICE_RESET" },
            { 0x887A000A, "DXGI_ERROR_WAS_STILL_DRAWING" },
            { 0x887A000B, "DXGI_ERROR_FRAME_STATISTICS_DISJOINT" },
            { 0x887A000C, "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE" },
            { 0x887A0020, "DXGI_ERROR_DRIVER_INTERNAL_ERROR" },
            { 0x887A0021, "DXGI_ERROR_NONEXCLUSIVE" },
            { 0x887A0022, "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE" },
            { 0x887A0023, "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED" },
            { 0x887A0024, "DXGI_ERROR_REMOTE_OUTOFMEMORY" },
            { 0x887A0026, "DXGI_ERROR_ACCESS_LOST" },
            { 0x887A0027, "DXGI_ERROR_WAIT_TIMEOUT" },
            { 0x887A0028, "DXGI_ERROR_SESSION_DISCONNECTED" },
            { 0x887A0029, "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE" },
            { 0x887A002A, "DXGI_ERROR_CANNOT_PROTECT_CONTENT" },
            { 0x887A002B, "DXGI_ERROR_ACCESS_DENIED" },
            { 0x887A002C, "DXGI_ERROR_NAME_ALREADY_EXISTS" },
            { 0x887A002D, "DXGI_ERROR_SDK_COMPONENT_MISSING" },
        };
    };
}
