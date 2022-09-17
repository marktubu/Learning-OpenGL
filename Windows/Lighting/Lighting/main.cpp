#include "app.h"
#include "blending.h"
#include "framebuffer.h"
#include "cubemap.h"
#include "ubo.h"

int main() {
	auto app = new UBO();
	app->InitWindow();
	app->Run();
}