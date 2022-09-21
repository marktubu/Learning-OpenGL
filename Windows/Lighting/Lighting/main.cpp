#include "app.h"
#include "blending.h"
#include "framebuffer.h"
#include "cubemap.h"
#include "ubo.h"
#include "geometry.h"
#include "instancing.h"
#include "antialiasing.h"

int main() {
	auto app = new Antialiasing();
	app->InitWindow();
	app->Run();
}