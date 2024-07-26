#pragma once

#ifdef SHADER_MNG_USES_WINDOWS
#ifdef SAMPLE_SHADER_MNG_DLL_MACRO_EXPORT
#define SAMPLE_SHADER_MNG_DLL_COMMAND __declspec(dllexport)
#elif SAMPLE_SHADER_MNG_DLL_MACRO_IMPORT
#define SAMPLE_SHADER_MNG_DLL_COMMAND __declspec(dllimport)
#else
#define SAMPLE_SHADER_MNG_DLL_COMMAND
#endif 
#endif
