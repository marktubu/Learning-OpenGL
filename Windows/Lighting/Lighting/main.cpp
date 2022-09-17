#include "app.h"
#include "blending.h"
#include "framebuffer.h"
#include "cubemap.h"
#include "ubo.h"
#include "geometry.h"

int main() {
	auto app = new Geometry();
	app->InitWindow();
	app->Run();
}