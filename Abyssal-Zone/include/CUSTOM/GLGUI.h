#pragma once
#include <vector>
#include "CUSTOM/renderer.h"
#include <string>



class Text {
public:
	float relx, rely, size;
	std::string text;
	Text(float relx, float rely, std::string text, float size) : relx(relx), rely(rely), text(text), size(size) {}
};
class MenuButton {
public:
	float relx, rely, size;
	int ID;
	Text text;
	MenuButton(float relx, float rely, int ID, float size, std::string t) : relx(relx), rely(rely), ID(ID), size(size), text(Text(relx, rely, t, size * 0.5f)) {
	}
private:

};

class MenuWindow {	
public:
	Renderer* renderer;

	RenderLayer buttonRenderer;
	std::vector<MenuButton> buttons;
	int buttonsInTexture;

	RenderLayer textRenderer;
	std::vector<Text> texts;
	int textInTexture = 19;

	MenuWindow(std::vector<MenuButton> buttons, int buttonsInTexture, std::vector<Text> texts, Renderer* renderer) :
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
		std::vector<Text> textsToDraw;
		for (Text text : texts) {
			textsToDraw.push_back(text);
		}
		for (MenuButton button : buttons) {
			textsToDraw.push_back(button.text);
		}
		buttonTriangleCount = 0;
		textTriangleCount = 0;
		int returnID = -1;
		size_t size = buttons.size() * 30;
		float* buttonArray = new float[size];
		size_t index = 0;
		for (int i = 0; i < buttons.size(); i++) {
			MenuButton button = buttons[i];
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
			buttonArray[index++] = 0.5f;
			buttonArray[index++] = isHover;

			// Bottom Right
			buttonArray[index++] = rx;
			buttonArray[index++] = by;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = 0.5f;
			buttonArray[index++] = isHover;

			// Top Left
			buttonArray[index++] = lx;
			buttonArray[index++] = uy;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = isHover;

			// Top Right
			buttonArray[index++] = rx;
			buttonArray[index++] = uy;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = isHover;

			// Bottom Right
			buttonArray[index++] = rx;
			buttonArray[index++] = by;
			buttonArray[index++] = 1.0f;
			buttonArray[index++] = 0.5f;
			buttonArray[index++] = isHover;

			// Top Left
			buttonArray[index++] = lx;
			buttonArray[index++] = uy;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = 0.0f;
			buttonArray[index++] = isHover;
		}
		buttonRenderer.setVertices(buttonArray, buttonTriangleCount, 15, GL_DYNAMIC_DRAW);
		buttonRenderer.setFloat("offset", buttonSize);
		delete[] buttonArray;

		// For text: relx, rely, tx, ty
		size = 0; // 24 floats per char, per Text
		for (Text text : textsToDraw) {
			for (char c : text.text) {
				size += 24;
			}
		}
		float* textArray = new float[size];
		index = 0;
		for (int i = 0; i < textsToDraw.size(); i++) {
			float xOffset = textsToDraw[i].text.size() * textsToDraw[i].size * -0.5f;
			for (int j = 0; j < textsToDraw[i].text.size(); j++) {
				if (textsToDraw[i].text[j] != ' ') { 
					int idx = fontCharacters.find(textsToDraw[i].text[j]);
					float texOffsetY = floorf(idx * 0.5f) * textSize;
					float texOffsetX = (idx % 2) * 0.5f;
					float lx = xOffset + textsToDraw[i].relx - textsToDraw[i].size * 0.5f;
					float rx = xOffset + textsToDraw[i].relx + textsToDraw[i].size * 0.5f;
					float uy = textsToDraw[i].rely + textsToDraw[i].size * 1.6f;
					float by = textsToDraw[i].rely - textsToDraw[i].size * 1.6f;
					textTriangleCount += 2;

					// Top left
					textArray[index++] = lx;
					textArray[index++] = uy;
					textArray[index++] = texOffsetX;
					textArray[index++] = texOffsetY;

					// Bottom left
					textArray[index++] = lx;
					textArray[index++] = by;
					textArray[index++] = texOffsetX;
					textArray[index++] = texOffsetY + textSize;

					// Bottom right
					textArray[index++] = rx;
					textArray[index++] = by;
					textArray[index++] = texOffsetX + 0.5f;
					textArray[index++] = texOffsetY + textSize;

					// Bottom right
					textArray[index++] = rx;
					textArray[index++] = by;
					textArray[index++] = texOffsetX + 0.5f;
					textArray[index++] = texOffsetY + textSize;

					// Top left
					textArray[index++] = lx;
					textArray[index++] = uy;
					textArray[index++] = texOffsetX;
					textArray[index++] = texOffsetY;

					// Top right
					textArray[index++] = rx;
					textArray[index++] = uy;
					textArray[index++] = texOffsetX + 0.5f;
					textArray[index++] = texOffsetY;
				}

				xOffset += textsToDraw[i].size;
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
	std::string fontCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:";
};
