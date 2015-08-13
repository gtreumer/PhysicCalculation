//#include <windows.h>
#include "App.h"

const int WIDTH = 640;
const int HEIGHT = 480;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	Test::App app(hInstance, nCmdShow, WIDTH, HEIGHT);
	app.start();

	return 0;
}