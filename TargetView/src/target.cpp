#include <iostream>
#include <Application.hpp>
#include "Entrypoint.hpp"

#include <Console.hpp>

#ifdef WIN32
#include <windows.h>

void ReadCmdArg(std::string* output, const char* argv);

#endif



#ifdef WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	#ifdef WIN32
	auto cmdArgs = GetCommandLineA();
	std::string programPath;
	ReadCmdArg(&programPath, (const char*)cmdArgs);
	SampleRender::Application* app = new SampleRender::Application(programPath);
	#else
	SampleRender::Application* app = new SampleRender::Application(argv[0]);
	#endif

	app->Run();
	delete app;
	return 0;
}

#ifdef WIN32
void ReadCmdArg(std::string* output, const char* argv)
{
	std::istringstream buffer(argv);
	buffer >> *output;
	size_t pos = 0;
	while ((pos = output->find("\"", pos)) != std::string::npos) {
		output->erase(pos, 1);
	}
}
#endif
