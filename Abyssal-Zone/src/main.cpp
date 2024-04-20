#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "CUSTOM/tilemap.h"
#include <iostream>
#include "CUSTOM/renderer.h"
#include <tuple>
using namespace std;

int windowWidth = 800;
int windowHeight = 600;

int main() {

	Renderer renderer(windowWidth, windowHeight, "The Abyssal Zone");
	RenderLayer tilemapRenderer({ 2, 2 }, "tile", "tile_texture", true); // x, y, tx, ty
	//RenderLayer playerRenderer({ 2, 2 }, "player_shader", "player_texture", true); // x, y, tx, ty
	//vector<vector<int>> tilemap = loadTilemap(0);
	//float* tilemapVertices;
	//int numTriangles;
	//std::tie(tilemapVertices, numTriangles) = tilemapDecoder(tilemap, 32, windowWidth, windowHeight);
	//tilemapRenderer.setVertices(tilemapVertices, numTriangles, 12, GL_DYNAMIC_DRAW);
	float testVertices[12] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	tilemapRenderer.setVertices(testVertices, 1, 12, GL_DYNAMIC_DRAW);
	
	glfwSwapInterval(1);
	while (renderer.isRunning()) {
		renderer.fillScreen(25, 20, 90);
		tilemapRenderer.draw(1);

		renderer.getDeltaTime();
		renderer.updateDisplay();
	}

	cout << "Hello, world!";
	return 0;
}