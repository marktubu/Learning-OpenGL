#include "app.h"
#include "blending.h"

int main() {
	auto app = new Blending();
	app->InitWindow();
	app->Run();
}