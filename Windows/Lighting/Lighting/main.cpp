#include "app.h"

int main() {
	App* app = new App();
	app->InitWindow();
	app->Run();
}