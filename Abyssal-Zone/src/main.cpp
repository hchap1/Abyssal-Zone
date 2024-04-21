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
	RenderLayer tilemapRenderer({ 2, 2, 1 }, "tile", "blocks", false); // x, y, tx, ty
	//RenderLayer playerRenderer({ 2, 2 }, "player_shader", "player_texture", true); // x, y, tx, ty
	vector<vector<int>> tilemap = loadTilemap(0);
	tuple<float*, int> vertexData = tilemapDecoder(tilemap, 32, windowWidth, windowHeight);
	tilemapRenderer.setVertices(get<0>(vertexData), get<1>(vertexData), 15, GL_STATIC_DRAW);

	float x_offset = 0.0f;
	float y_offset = 0.0f;

	glfwSwapInterval(1);
	while (renderer.isRunning()) {
		float dt = renderer.getDeltaTime();
		renderer.fillScreen(25, 20, 90);

		tilemapRenderer.setFloat("xOffset", x_offset);
		tilemapRenderer.setFloat("yOffset", y_offset);
		tilemapRenderer.draw(get<1>(vertexData));

		if (renderer.getKeyDown(GLFW_KEY_UP)) {
			y_offset -= 0.0001f * dt;
		}
		if (renderer.getKeyDown(GLFW_KEY_DOWN)) {
			y_offset += 0.0001f * dt;
		}
		renderer.updateDisplay();
	}

	cout << "Hello, world!";
	return 0;
}