#pragma once
#include "CUSTOM/renderer.h"
#include <vector>
using namespace std;

class Button {
public:
	float relx, rely, size;
	int ID;
	Button(float relx, float rely, int ID, float size) : relx(relx), rely(rely), ID(ID), size(size) {

	}
};

class Window {
	RenderLayer backgroundRenderer;
	RenderLayer buttonRenderer;
	Window(vector<Button> buttons, string buttonTextureName, int buttonsInTexture) {
		// X,Y,TX,TY
		// Plan is to have mousex, mousey passed into shader to
		// do shading.
		// Total (incl hover colours)
		float buttonSize = 1.0f / buttonsInTexture;
		size_t size = buttons.size() * 24;
		float* buttonArray = new float[size];
		size_t index = 0;
		for (int i = 0; i < buttons.size(); i++) {
			Button button = buttons[i];
			float ty = buttonSize * button.ID * 2;

			// Bottom Left
			buttonArray[index++] = button.relx - button.size * 2.0f;
			buttonArray[index++] = button.rely - button.size * 0.5f;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = ty;

			// Bottom Right
			buttonArray[index++] = button.relx + button.size * 2.0f;
			buttonArray[index++] = button.rely - button.size * 0.5f;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = ty;

			// Top Left
			buttonArray[index++] = button.relx - button.size * 2.0f;
			buttonArray[index++] = button.rely + button.size * 0.5f;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = ty + buttonSize;

			// Top Right
			buttonArray[index++] = button.relx + button.size * 2.0f;
			buttonArray[index++] = button.rely + button.size * 0.5f;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = ty + buttonSize;

			// Bottom Right
			buttonArray[index++] = button.relx + button.size * 2.0f;
			buttonArray[index++] = button.rely - button.size * 0.5f;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = ty;

			// Top Left
			buttonArray[index++] = button.relx - button.size * 2.0f;
			buttonArray[index++] = button.rely + button.size * 0.5f;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = ty + buttonSize;
		}

		RenderLayer buttonRenderer({ 2, 2 }, "button", buttonTextureName, true);
		buttonRenderer.setVertices(buttonArray, buttons.size() * 2, 24, GL_STATIC_DRAW);
	}

	void draw(Renderer* renderer) {
		buttonRenderer.setFloat("mouseX", renderer->mouseX);
		buttonRenderer.setFloat("mouseY", renderer->mouseY);
	}
};

