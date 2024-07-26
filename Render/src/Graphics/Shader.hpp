#pragma once

namespace SampleRender
{
	class Shader
	{
	public:
		Shader();
		~Shader();

		void update();

		static Shader* Instantiate();
	};
}