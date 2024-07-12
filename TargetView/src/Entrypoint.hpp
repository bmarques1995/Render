#ifdef WIN32
	#include <windows.h>
	#define entrypoint int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
	#define entrypoint int main(int argc, char** argv)
#endif