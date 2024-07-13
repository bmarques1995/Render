#pragma once

#include "Layer.hpp"
#include <list>

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::list<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::list<Layer*>::iterator end() { return m_Layers.end(); }

	private:
		std::list<Layer*> m_Layers;
		std::list<Layer*>::iterator m_LayerInsert;

	};
}