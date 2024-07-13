#include <iostream>
#include <Application.hpp>
#include "Entrypoint.hpp"

#include <Console.hpp>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	SampleRender::Application* app = new SampleRender::Application();
	SampleRender::Console::Log("Sample Client Log");
	app->Run();
	delete app;
	return 0;
}
