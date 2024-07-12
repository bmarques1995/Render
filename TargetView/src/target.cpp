#include <iostream>
#include <Application.hpp>
#include "Entrypoint.hpp"

entrypoint
{
	SampleRender::Application* app = new SampleRender::Application();
	app->Run();
	delete app;
	return 0;
}
