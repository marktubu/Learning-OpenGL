#include "app.h"
#include "blending.h"
#include "framebuffer.h"
#include "cubemap.h"
#include "ubo.h"
#include "geometry.h"
#include "instancing.h"

int main() {
	auto app = new Instancing();
	app->InitWindow();
	app->Run();
}