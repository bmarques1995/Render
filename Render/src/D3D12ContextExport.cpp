#ifdef RENDER_USES_WINDOWS

#include <cstdint>

extern "C"
{
	__declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614;
	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

#endif
