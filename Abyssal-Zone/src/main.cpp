#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "CUSTOM/tilemap.h"
#include <iostream>
#include "CUSTOM/renderer.h"
#include <tuple>
using namespace std;

int windowWidth = 1920;
int windowHeight = 1080;

int main() {

	Renderer renderer(windowWidth, windowHeight, "The Abyssal Zone");
	RenderLayer tilemapRenderer({ 2, 2, 1 }, "tile", "tile_texture", false); // x, y, tx, ty
	RenderLayer backgroundRenderer({ 2, 2, 1 }, "background", "tile_texture", false); // x, y, tx, ty
	//RenderLayer playerRenderer({ 2, 2 }, "player_shader", "player_texture", true); // x, y, tx, ty
	tuple<vector<vector<int>>, vector<vector<int>>> tilemaps = loadTilemap(1);
	tuple<float*, int> tilemapVertexData = tilemapDecoder(get<0>(tilemaps), 8, windowWidth, windowHeight);
	tuple<float*, int> backgroundVertexData = tilemapDecoder(get<1>(tilemaps), 8, windowWidth, windowHeight);
	tilemapRenderer.setVertices(get<0>(tilemapVertexData), get<1>(tilemapVertexData), 15, GL_STATIC_DRAW);
	backgroundRenderer.setVertices(get<0>(backgroundVertexData), get<1>(backgroundVertexData), 15, GL_STATIC_DRAW);

	float x_offset = 0.0f;
	float y_offset = 0.0f;

	tilemapRenderer.setFloat("screenX", windowWidth);
	tilemapRenderer.setFloat("screenY", windowHeight);
	backgroundRenderer.setFloat("screenX", windowWidth);
	backgroundRenderer.setFloat("screenY", windowHeight);

	glfwSwapInterval(1);
	while (renderer.isRunning()) {
		float dt = renderer.getDeltaTime();
		float fps = 1.0f / dt;
		renderer.fillScreen(0, 0, 0);

		backgroundRenderer.setFloat("xOffset", x_offset);
		backgroundRenderer.setFloat("yOffset", y_offset);
		backgroundRenderer.draw(get<1>(backgroundVertexData));

		tilemapRenderer.setFloat("xOffset", x_offset);
		tilemapRenderer.setFloat("yOffset", y_offset);
		tilemapRenderer.draw(get<1>(tilemapVertexData));

		if (renderer.getKeyDown(GLFW_KEY_UP)) {
			y_offset -= 1.0f * dt;
		}
		if (renderer.getKeyDown(GLFW_KEY_DOWN)) {
			y_offset += 1.0f * dt;
		}
		if (renderer.getKeyDown(GLFW_KEY_LEFT)) {
			x_offset -= 1.0f * dt;
		}
		if (renderer.getKeyDown(GLFW_KEY_RIGHT)) {
			x_offset += 1.0f * dt;
		}
		renderer.updateDisplay();
	}

	cout << "Hello, world!";
	return 0;
}