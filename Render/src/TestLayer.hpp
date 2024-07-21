#pragma once

#include "Layer.hpp"
#include <windows.h>
#include "ComPointer.hpp"
#include <dxgi1_6.h>
#include <d3d12.h>
#ifdef RENDER_DEBUG_MODE
#include <dxgidebug.h>
#include <d3d12sdklayers.h>
#endif

namespace SampleRender
{


	class SAMPLE_RENDER_DLL_COMMAND TestLayer : public Layer
	{
	public:
		TestLayer();
		~TestLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		
	};
}
