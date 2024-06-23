#pragma once
#include <vector>
#include "CUSTOM/renderer.h"
#include <string>
using namespace std;

class MenuButton {
public:
	float relx, rely, size;
	int ID;
	MenuButton(float relx, float rely, int ID, float size) : relx(relx), rely(rely), ID(ID), size(size) {}
private:

};
class Text {
public:
	float relx, rely, size;
	string text;
	Text(float relx, float rely, string text, float size) : relx(relx), rely(rely), text(text), size(size) {}
};

class MenuWindow {	
public:
	Renderer* renderer;

	RenderLayer buttonRenderer;
	vector<MenuButton> buttons;
	int buttonsInTexture;

	RenderLayer textRenderer;
	vector<Text> texts;
	int textInTexture = 26;

	MenuWindow(vector<MenuButton> buttons, int buttonsInTexture, vector<Text> texts, Renderer* renderer) : 
		renderer(renderer), 
		buttonRenderer({ 2, 2, 1 }, "button", "buttons", false), buttons(buttons), buttonsInTexture(buttonsInTexture),
		textRenderer({ 2, 2 }, "text", "font", false), texts(texts) {
		buttonTriangleCount = 0;
		textTriangleCount = 0;
		// X,Y,TX,TY
		// Plan is to have mousex, mousey passed into shader to
		// do shading.
		// Total (incl hover colours)
		buttonSize = 1.0f / buttonsInTexture;
		textSize = 1.0f / textInTexture;
	}

	int setArray(float mouseX, float mouseY) {
		buttonTriangleCount = 0;
		textTriangleCount = 0;
		int returnID = -1;
		size_t size = buttons.size() * 30;
		float* buttonArray = new float[size];
		size_t index = 0;
		for (int i = 0; i < buttons.size(); i++) {
			MenuButton button = buttons[i];
			float ty = buttonSize * (button.ID * 2);
			buttonTriangleCount += 2;
			float lx = button.relx - button.size * 3.0f;
			float rx = button.relx + button.size * 3.0f;
			float uy = button.rely + button.size * 1.0f;
			float by = button.rely - button.size * 1.0f;
			float isHover = 0.0f;

			if (mouseX > lx && mouseX < rx) {
				if (mouseY > by && mouseY < uy) {
					isHover = 1.0f;
					returnID = button.ID;
				}
			}

			// Bottom Left
			buttonArray[index++] = lx;
			buttonArray[index++] = by;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = ty + buttonSize;
			buttonArray[index++] = isHover;

			// Bottom Right
			buttonArray[index++] = rx;
			buttonArray[index++] = by;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = ty + buttonSize;
			buttonArray[index++] = isHover;

			// Top Left
			buttonArray[index++] = lx;
			buttonArray[index++] = uy;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = ty;
			buttonArray[index++] = isHover;

			// Top Right
			buttonArray[index++] = rx;
			buttonArray[index++] = uy;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = ty;
			buttonArray[index++] = isHover;

			// Bottom Right
			buttonArray[index++] = rx;
			buttonArray[index++] = by;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = ty + buttonSize;
			buttonArray[index++] = isHover;

			// Top Left
			buttonArray[index++] = lx;
			buttonArray[index++] = uy;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = ty;
			buttonArray[index++] = isHover;
		}
		buttonRenderer.setVertices(buttonArray, buttonTriangleCount, 15, GL_DYNAMIC_DRAW);
		buttonRenderer.setFloat("offset", buttonSize);
		delete[] buttonArray;

		// For text: relx, rely, tx, ty
		size = 0; // 24 floats per char, per Text
		for (Text text : texts) {
			for (char c : text.text) {
				size += 24;
			}
		}
		float* textArray = new float[size];
		index = 0;
		for (int i = 0; i < texts.size(); i++) {
			float xOffset = texts[i].text.size() * texts[i].size * -0.5f;
			for (int j = 0; j < texts[i].text.size(); j++) {
				float texOffset = fontCharacters.find(texts[i].text[j]) * textSize;
				float lx = xOffset + texts[i].relx - texts[i].size * 0.5f;
				float rx = xOffset + texts[i].relx + texts[i].size * 0.5f;
				float uy = texts[i].rely + texts[i].size * 1.6f;
				float by = texts[i].rely - texts[i].size * 1.6f;
				textTriangleCount += 2;

				// Top left
				textArray[index++] = lx;
				textArray[index++] = uy;
				textArray[index++] = 0.0f;
				textArray[index++] = texOffset;

				// Bottom left
				textArray[index++] = lx;
				textArray[index++] = by;
				textArray[index++] = 0.0f;
				textArray[index++] = texOffset + textSize;

				// Bottom right
				textArray[index++] = rx;
				textArray[index++] = by;
				textArray[index++] = 1.0f;
				textArray[index++] = texOffset + textSize;

				// Bottom right
				textArray[index++] = rx;
				textArray[index++] = by;
				textArray[index++] = 1.0f;
				textArray[index++] = texOffset + textSize;

				// Top left
				textArray[index++] = lx;
				textArray[index++] = uy;
				textArray[index++] = 0.0f;
				textArray[index++] = texOffset;

				// Top right
				textArray[index++] = rx;
				textArray[index++] = uy;
				textArray[index++] = 1.0f;
				textArray[index++] = texOffset;

				xOffset += texts[i].size;
			}
		}

		textRenderer.setVertices(textArray, textTriangleCount, 12, GL_DYNAMIC_DRAW);
		delete[] textArray;

		return returnID;
	}

	int draw(float mouseX, float mouseY) {
		int returnID = setArray(mouseX, mouseY);
		buttonRenderer.draw(buttonTriangleCount);
		textRenderer.draw(textTriangleCount);
		return returnID;

	}

private:
	int textTriangleCount;
	int buttonTriangleCount;
	float buttonSize;
	float textSize;
	string fontCharacters = "ABCDEFGH0123456789.:";
};
