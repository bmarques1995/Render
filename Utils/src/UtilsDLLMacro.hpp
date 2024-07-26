#pragma once

#ifdef UTILS_USES_WINDOWS
#ifdef SAMPLE_UTILS_DLL_MACRO_EXPORT
#define SAMPLE_UTILS_DLL_COMMAND __declspec(dllexport)
#elif SAMPLE_UTILS_DLL_MACRO_IMPORT
#define SAMPLE_UTILS_DLL_COMMAND __declspec(dllimport)
#else
#define SAMPLE_UTILS_DLL_COMMAND
#endif 
#endif
