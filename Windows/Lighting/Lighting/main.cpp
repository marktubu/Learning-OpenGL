#include "app.h"
#include "blending.h"
#include "framebuffer.h"

int main() {
	auto app = new FrameBuffer();
	app->InitWindow();
	app->Run();
}