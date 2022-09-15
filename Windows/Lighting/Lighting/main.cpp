#include "app.h"
#include "blending.h"
#include "framebuffer.h"
#include "cubemap.h"

int main() {
	auto app = new CubeMap();
	app->InitWindow();
	app->Run();
}