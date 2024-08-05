#include "Shader.hpp"
#include "Application.hpp"
#ifdef RENDER_USES_WINDOWS
#include "D3D12Shader.hpp"
#include "D3D12Context.hpp"
#endif
#include "VKContext.hpp"
#include "VKShader.hpp"

SampleRender::Shader* SampleRender::Shader::Instantiate(const std::shared_ptr<GraphicsContext>* context, std::string json_basepath, BufferLayout layout)
{
	GraphicsAPI api = Application::GetInstance()->GetCurrentAPI();
	std::stringstream controller_path;
	controller_path << json_basepath;
	switch (api)
	{
#ifdef RENDER_USES_WINDOWS
	case SampleRender::SAMPLE_RENDER_GRAPHICS_API_D3D12:
	{
		controller_path << ".d3d12.json";
		std::string json_controller_path = controller_path.str();
		return new D3D12Shader((const std::shared_ptr<D3D12Context>*)(context), json_controller_path, layout);
	}
#endif
	case SampleRender::SAMPLE_RENDER_GRAPHICS_API_VK:
	{
		controller_path << ".vk.json";
		std::string json_controller_path = controller_path.str();
		return new VKShader((const std::shared_ptr<VKContext>*)(context), json_controller_path, layout);
	}
	default:
		break;
	}
	return nullptr;
}
