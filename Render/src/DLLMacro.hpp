#pragma once

#ifdef SAMPLE_RENDER_DLL_MACRO_EXPORT
	#define SAMPLE_RENDER_DLL_COMMAND __declspec(dllexport)
#elif SAMPLE_RENDER_DLL_MACRO_IMPORT
	#define SAMPLE_RENDER_DLL_COMMAND __declspec(dllimport)
#else
	#define SAMPLE_RENDER_DLL_COMMAND
#endif // DEBUG
