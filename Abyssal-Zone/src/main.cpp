#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "CUSTOM/tilemap.h"
#include <iostream>
#include "CUSTOM/renderer.h"
#include <tuple>
#include <random>
#include "CUSTOM/network.h"
#include "CUSTOM/GLGUI.h"
#include <windows.h>

using namespace std;

int windowWidth = 1920;
int windowHeight = 1080;
float startX = 3.0f;
float startY = 2.0f;
float playerWidth = 0.8f;
float playerHeight = 1.8f;
float blockSize = 300.0f;

bool collide(int blockID) {
	if (blockID == 1 || blockID == 4) {
		return true;
	}
	return false;
}

bool ladder(int blockID) {
	return blockID == 6;
}

char getCharacterFromGLFWKeyCode(int glfwKeyCode) {
	if (glfwKeyCode >= GLFW_KEY_A && glfwKeyCode <= GLFW_KEY_H) {
		return static_cast<char>('A' + (glfwKeyCode - GLFW_KEY_A));
	}
	else if (glfwKeyCode >= GLFW_KEY_0 && glfwKeyCode <= GLFW_KEY_9) {
		return static_cast<char>('0' + (glfwKeyCode - GLFW_KEY_0));
	}
	if (glfwKeyCode == GLFW_KEY_PERIOD) {
		return '.';
	}
	if (glfwKeyCode == GLFW_KEY_SEMICOLON) {
		return ':';
	}
	return '\0';
}

tuple<int, string> GUI(Renderer* renderer, vector<MenuButton> buttons, vector<Text> texts, string pageID) {

	bool running = true;
	MenuWindow window(buttons, 8, texts, renderer);
	vector<int> keyTracker;
	int backspaceTracker = 0;
	vector<int> glfwKeyCodes;
	for (int i = GLFW_KEY_A; i <= GLFW_KEY_H; ++i) {
		glfwKeyCodes.push_back(i);
		keyTracker.push_back(0);
	}
	for (int i = GLFW_KEY_0; i <= GLFW_KEY_9; ++i) {
		keyTracker.push_back(0);
		glfwKeyCodes.push_back(i);
	}
	glfwKeyCodes.push_back(GLFW_KEY_PERIOD);
	glfwKeyCodes.push_back(GLFW_KEY_SEMICOLON);
	keyTracker.push_back(0);
	keyTracker.push_back(0);

	glfwSwapInterval(1);
	while (running) {
		bool mouseDown = renderer->mouseWasJustClicked();
		if (pageID == "multiplayer" || pageID == "join") {
			for (int i = 0; i < keyTracker.size(); i++) {
				if (renderer->getKeyDown(glfwKeyCodes[i])) {
					keyTracker[i]++;
				}
				else {
					keyTracker[i] = 0;
				}
				if (keyTracker[i] == 1) {
					window.texts[0].text += getCharacterFromGLFWKeyCode(glfwKeyCodes[i]);
				}
			}
			if (renderer->getKeyDown(GLFW_KEY_BACKSPACE)) {
				backspaceTracker++;
			}
			else {
				backspaceTracker = 0;
			}
			if (backspaceTracker == 1) {
				window.texts[0].text = window.texts[0].text.substr(0, window.texts[0].text.size() - 1);
			}

		}
		renderer->fillScreen(255, 255, 255);
		int buttonPressed = window.draw(renderer->mouseX, renderer->mouseY);
		if (buttonPressed != -1) {
			if (mouseDown) {
				if (pageID == "multiplayer" || pageID == "join") {
					return make_tuple(buttonPressed, window.texts[0].text);
				}
				else {
					return make_tuple(buttonPressed, "");
				}
			}
		}
		renderer->updateDisplay();
	}
	return make_tuple(-1, "");
}

int game(string joinCode, Renderer* renderer, string ID) {
	float timeUntilFlicker = 3.0f;
	float flickerDuration = 0.5f;
	float flickerTimer = 0.0f;

	float relativeFPS;
	float dt;
	float movementMultiplier = 300.0f / blockSize;

	float halfPlayerWidth = blockSize * 0.5f / windowWidth * playerWidth;
	float halfPlayerHeight = blockSize * 0.5f / windowHeight * playerHeight;

	float blockWidth = blockSize / windowWidth;
	float blockHeight = blockSize / windowHeight;
	float indexXRight;
	float indexXRightSmall;
	float indexXLeft;
	float indexXLeftSmall;
	float indexY;
	float indexHeadY;
	float indexTop;
	float indexMiddle;
	float animationCycle = 0.0f;
	float cycleCount = 0.0f;
	float lightScale = 1.0f;

	bool crouching = false;
	int jumpKeyCounter = 0;
	int multiplayerTriangleCount = 0;
	float t = 0;

	float playerX;
	float playerY;
	float zoom = 0.5f;
	float frameTimer = 0.1f;
	float frame = 0.0f;
	float dir;

	int health = 100;

	bool RCV = false;
	bool doPhysics = false;
	string RCV_str = "";
	
	dt = renderer->getDeltaTime();

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> distribution(1, 10);
	uniform_int_distribution<int> distribution2(80, 100);

	RenderLayer tilemapRenderer({ 2, 2, 1 }, "tile", "tile_texture", false); // vx, vy, tx, ty, lx, ly
	RenderLayer playerRenderer({ 2, 2 }, "player", "player_texture", true);
	RenderLayer multiplayerRenderer({ 2, 2, 1, 1, 1 }, "multiplayer", "player_texture", true);
	RenderLayer enemyRenderer({ 2, 2 }, "enemy", "enemies", true);

	bool doMultiplayer = false;
	thread recvThread;
	thread sendThread;
	Client client;

	tuple<vector<vector<int>>, float(*)[4], int> tilemapData = loadTilemapFromFile(1);
	vector<vector<int>> tilemap = get<0>(tilemapData);

	if (joinCode != "NONE") {
		client = Client(joinCode, halfPlayerWidth, halfPlayerHeight, &playerX, &playerY, &crouching, &frame, 
			&dir, ID, &RCV, &RCV_str, blockWidth, blockHeight, &health);
		doMultiplayer = true;
		recvThread = thread(&Client::recvData, &client);
		sendThread = thread(&Client::sendData, &client);
	}

	tuple<float*, int, float, float> tilemapVertexData = tilemapDecoder(tilemap, 14, windowWidth, windowHeight, blockSize);
	//tilemapRenderer.setVertices(get<0>(tilemapVertexData), get<1>(tilemapVertexData), 15, GL_STATIC_DRAW);
	//delete[] get<0>(tilemapVertexData);
	tilemapRenderer.setFloat("blockX", get<2>(tilemapVertexData));
	tilemapRenderer.setFloat("blockY", get<3>(tilemapVertexData));
	tilemapRenderer.setFloat("texOffset", 1.0f / 14.0f);
	tilemapRenderer.setFloat("torchLight", 1.0f);
	playerRenderer.setFloat("blockX", get<2>(tilemapVertexData));
	playerRenderer.setFloat("blockY", get<3>(tilemapVertexData));
	playerRenderer.setFloat("texOffset", 1.0f / 14.0f);
	playerRenderer.setFloat("torchLight", 1.0f);
	multiplayerRenderer.setFloat("blockX", get<2>(tilemapVertexData));
	multiplayerRenderer.setFloat("blockY", get<3>(tilemapVertexData));
	multiplayerRenderer.setFloat("texOffset", 1.0f / 14.0f);
	multiplayerRenderer.setFloat("torchLight", 1.0f);
	enemyRenderer.setFloat("blockX", get<2>(tilemapVertexData));
	enemyRenderer.setFloat("blockY", get<3>(tilemapVertexData));
	enemyRenderer.setFloat("texOffset", 1.0f / 14.0f);
	enemyRenderer.setFloat("torchLight", 1.0f);


	playerX = -blockWidth * startX - halfPlayerWidth * 1.5f;
	playerY = -blockHeight * startY - halfPlayerHeight;

	dir = 0.0f;
	float playerXVel = 0.0f;
	float playerYVel = 0.0f;
	float grounded;
	float playerTexOffset = 1.0f / 10.0f;
	float playerVertexData[] = {
		-halfPlayerWidth, -halfPlayerHeight, 0.0f, 0.5f,
		-halfPlayerWidth,  halfPlayerHeight, 0.0f, 1.0f,
		 halfPlayerWidth, -halfPlayerHeight, playerTexOffset, 0.5f,
		 halfPlayerWidth,  halfPlayerHeight, playerTexOffset, 1.0f,
		-halfPlayerWidth,  halfPlayerHeight, 0.0f, 1.0f,
		 halfPlayerWidth, -halfPlayerHeight, playerTexOffset, 0.5f,
	};

	playerRenderer.setVertices(playerVertexData, 2, 12, GL_STATIC_DRAW);
	tilemapRenderer.setFloat("screenX", static_cast<float>(windowWidth));
	tilemapRenderer.setFloat("screenY", static_cast<float>(windowHeight));
	playerRenderer.setFloat("screenX", static_cast<float>(windowWidth));
	playerRenderer.setFloat("screenY", static_cast<float>(windowHeight));
	multiplayerRenderer.setFloat("screenX", static_cast<float>(windowWidth));
	multiplayerRenderer.setFloat("screenY", static_cast<float>(windowHeight));
	enemyRenderer.setFloat("screenX", static_cast<float>(windowWidth));
	enemyRenderer.setFloat("screenY", static_cast<float>(windowHeight));

	glfwSwapInterval(1);
	dt = renderer->getDeltaTime();
	while (renderer->isRunning()) {
		if (health <= 0) {
			break;
		}
		dt = renderer->getDeltaTime();
		frameTimer -= dt;
		cycleCount += dt;
		if (cycleCount > 0.1f) { animationCycle += 1.0f; cycleCount = 0.0f; tilemapRenderer.setFloat("torchLight", distribution2(gen) / 100.0f); }
		if (animationCycle > 7.0f) { animationCycle = 0.0f; }
		tilemapRenderer.setFloat("lightConstant", 1.0f);
		tilemapRenderer.setFloat("frame", animationCycle);
		playerRenderer.setFloat("lightConstant", 1.0f);
		playerRenderer.setFloat("frame", animationCycle);
		tilemapRenderer.setFloat("lightScale", lightScale);
		playerRenderer.setFloat("lightScale", lightScale);
		playerRenderer.setFloat("frame", frame);
		multiplayerRenderer.setFloat("lightScale", lightScale);
		multiplayerRenderer.setFloat("lightConstant", 1.0f);
		multiplayerRenderer.setFloat("frame", animationCycle);
		enemyRenderer.setFloat("lightScale", lightScale);
		enemyRenderer.setFloat("lightConstant", 1.0f);
		enemyRenderer.setFloat("frame", animationCycle);
		timeUntilFlicker -= dt;
		if (flickerTimer > 0.0f) {
			flickerTimer -= dt;
			if (flickerTimer > 0.2f) {
				tilemapRenderer.setFloat("lightConstant", 0.3f);
				playerRenderer.setFloat("lightConstant", 0.3f);
				multiplayerRenderer.setFloat("lightConstant", 0.3f);
				enemyRenderer.setFloat("lightConstant", 0.3f);
			}
			else if (flickerTimer > 0.1f) {
				tilemapRenderer.setFloat("lightConstant", 1.5f);
				playerRenderer.setFloat("lightConstant", 1.5f);
				multiplayerRenderer.setFloat("lightConstant", 1.5f);
				enemyRenderer.setFloat("lightConstant", 1.5f);
			}
			else {
				tilemapRenderer.setFloat("lightConstant", 0.1f);
				playerRenderer.setFloat("lightConstant", 0.1f);
				multiplayerRenderer.setFloat("lightConstant", 0.1f);
				enemyRenderer.setFloat("lightConstant", 0.1f);
			}
		}
		renderer->fillScreen(0, 0, 0);

		if (timeUntilFlicker <= 0.0f) {
			timeUntilFlicker = distribution(gen);
			flickerTimer = flickerDuration;
		}
		if (renderer->getKeyDown(GLFW_KEY_I)) {
			//zoom -= dt;
		}
		else if (renderer->getKeyDown(GLFW_KEY_O)) {
			//zoom += dt;
		}
		if (renderer->getKeyDown(GLFW_KEY_K)) {
			lightScale -= dt;
		}
		else if (renderer->getKeyDown(GLFW_KEY_L)) {
			lightScale += dt;
		}
		if (static_cast<int>(playerXVel * 100) != 0) { dir = abs(playerXVel) / -playerXVel; }

		if (RCV) {
			RCV = false;
			doPhysics = true;
			vector<string> coords = splitString(splitString(splitString(RCV_str, '|')[0], '>')[1], ',');
			startX = stof(coords[0]);
			startY = stof(coords[1]);
			playerX = -blockWidth * startX - halfPlayerWidth * 1.5f;
			playerY = -blockHeight * startY - halfPlayerHeight;
			tuple<vector<vector<int>>, float(*)[4], int> tilemapData = loadTilemapFromString(RCV_str);
			tilemap = get<0>(tilemapData);
			float(*lightArray)[4] = get<1>(tilemapData);
			int numLights = get<2>(tilemapData);
			if (numLights > 0) { 
				tilemapRenderer.setArray_64_vec4("lightSources", lightArray, numLights);
				playerRenderer.setArray_64_vec4("lightSources", lightArray, numLights);
				multiplayerRenderer.setArray_64_vec4("lightSources", lightArray, numLights); 
				enemyRenderer.setArray_64_vec4("lightSources", lightArray, numLights);
			}
			tilemapRenderer.setFloat("lightCount", static_cast<float>(numLights));
			playerRenderer.setFloat("lightCount", static_cast<float>(numLights));
			multiplayerRenderer.setFloat("lightCount", static_cast<float>(numLights));
			enemyRenderer.setFloat("lightCount", static_cast<float>(numLights));

			tuple<float*, int, float, float> tilemapVertexData = tilemapDecoder(tilemap, 14, windowWidth, windowHeight, blockSize);
			t = get<1>(tilemapVertexData);
			tilemapRenderer.setVertices(get<0>(tilemapVertexData), get<1>(tilemapVertexData), 15, GL_STATIC_DRAW);
			delete[] get<0>(tilemapVertexData);
		}

		tilemapRenderer.setFloat("zoom", zoom);
		tilemapRenderer.setFloat("xOffset", playerX);
		tilemapRenderer.setFloat("yOffset", playerY);
		tilemapRenderer.draw(t);
		playerRenderer.setFloat("zoom", zoom);
		playerRenderer.setFloat("xOffset", playerX);
		playerRenderer.setFloat("yOffset", playerY);
		playerRenderer.setFloat("direction", dir);
		if (playerXVel != 0.0f && frameTimer < 0.0f) {
			frame += 1.0f;
			frameTimer = 0.07f;
		}
		if ((frame >= 9.0f || frame < 2.0f) && playerXVel != 0.0f) {
			frame = 2.0f;
		}
		if (abs(playerXVel) < 0.05f) {
			frame = 0.0f;
		}
		if (playerYVel > 0.0f) {
			frame = 1.0f;
		}
		if (doMultiplayer) {
			multiplayerRenderer.setFloat("zoom", zoom);
			multiplayerRenderer.setFloat("xOffset", playerX);
			multiplayerRenderer.setFloat("yOffset", playerY);
			enemyRenderer.setFloat("zoom", zoom);
			enemyRenderer.setFloat("xOffset", playerX);
			enemyRenderer.setFloat("yOffset", playerY);
			
			map<string, PlayerData> playerData = client.multiplayerData;
			map<string, EnemyData> enemyData = client.enemyData;

			size_t validCount = 0;
			for (const auto& pair : playerData) {
				if (pair.first != ID) {
					validCount++;
				}
			}
			size_t size = validCount * 42;
			size_t triangleCount = 0;
			float* multiplayerVertexArray = new float[size];
			size_t index = 0;
			for (const auto& pair : playerData) {
				PlayerData player = pair.second;
				float xPos = player.x * blockWidth + halfPlayerWidth * 1.5f;
				float yPos = player.y * blockHeight + halfPlayerHeight;
				float crouching = 0.0f;
				float direction = player.direction;
				float mpFrame = player.frame;
				if (player.crouching) { crouching = 1.0f; }
				if (pair.first != ID) {
					triangleCount += 2;
					multiplayerVertexArray[index++] = xPos - halfPlayerWidth;
					multiplayerVertexArray[index++] = yPos + halfPlayerHeight;
					multiplayerVertexArray[index++] = 0.0f;
					multiplayerVertexArray[index++] = 1.0f;
					multiplayerVertexArray[index++] = crouching;
					multiplayerVertexArray[index++] = mpFrame;
					multiplayerVertexArray[index++] = direction;

					multiplayerVertexArray[index++] = xPos - halfPlayerWidth;
					multiplayerVertexArray[index++] = yPos - halfPlayerHeight;
					multiplayerVertexArray[index++] = 0.0f;
					multiplayerVertexArray[index++] = 0.5f;
					multiplayerVertexArray[index++] = crouching;
					multiplayerVertexArray[index++] = mpFrame;
					multiplayerVertexArray[index++] = direction;

					multiplayerVertexArray[index++] = xPos + halfPlayerWidth;
					multiplayerVertexArray[index++] = yPos - halfPlayerHeight;
					multiplayerVertexArray[index++] = playerTexOffset;
					multiplayerVertexArray[index++] = 0.5f;
					multiplayerVertexArray[index++] = crouching;
					multiplayerVertexArray[index++] = mpFrame;
					multiplayerVertexArray[index++] = direction;

					multiplayerVertexArray[index++] = xPos + halfPlayerWidth;
					multiplayerVertexArray[index++] = yPos + halfPlayerHeight;
					multiplayerVertexArray[index++] = playerTexOffset;
					multiplayerVertexArray[index++] = 1.0f;
					multiplayerVertexArray[index++] = crouching;
					multiplayerVertexArray[index++] = mpFrame;
					multiplayerVertexArray[index++] = direction;

					multiplayerVertexArray[index++] = xPos - halfPlayerWidth;
					multiplayerVertexArray[index++] = yPos + halfPlayerHeight;
					multiplayerVertexArray[index++] = 0.0f;
					multiplayerVertexArray[index++] = 1.0f;
					multiplayerVertexArray[index++] = crouching;
					multiplayerVertexArray[index++] = mpFrame;
					multiplayerVertexArray[index++] = direction;

					multiplayerVertexArray[index++] = xPos + halfPlayerWidth;
					multiplayerVertexArray[index++] = yPos - halfPlayerHeight;
					multiplayerVertexArray[index++] = playerTexOffset;
					multiplayerVertexArray[index++] = 0.5f;
					multiplayerVertexArray[index++] = crouching;
					multiplayerVertexArray[index++] = mpFrame;
					multiplayerVertexArray[index++] = direction;
				}
			}
			multiplayerRenderer.setVertices(multiplayerVertexArray, triangleCount, 21, GL_DYNAMIC_DRAW);
			multiplayerRenderer.draw(triangleCount);
			delete[] multiplayerVertexArray;

			validCount = 0;
			for (const auto& pair : enemyData) {
				validCount += 1;
			}
			triangleCount = 0;
			float* enemyVertexArray = new float[validCount * 24];
			index = 0;
			halfPlayerHeight *= 0.5f;
			for (const auto& pair : enemyData) {
				EnemyData enemy = pair.second;
				float xPos = enemy.x * blockWidth + halfPlayerWidth * 1.5f;
				float yPos = enemy.y * blockHeight + halfPlayerHeight;
				triangleCount += 2;
				enemyVertexArray[index++] = xPos - halfPlayerWidth;
				enemyVertexArray[index++] = yPos + halfPlayerHeight;
				enemyVertexArray[index++] = 0.0f;
				enemyVertexArray[index++] = 1.0f;

				enemyVertexArray[index++] = xPos - halfPlayerWidth;
				enemyVertexArray[index++] = yPos - halfPlayerHeight;
				enemyVertexArray[index++] = 0.0f;
				enemyVertexArray[index++] = 0.0f;

				enemyVertexArray[index++] = xPos + halfPlayerWidth;
				enemyVertexArray[index++] = yPos - halfPlayerHeight;
				enemyVertexArray[index++] = 1.0f;;
				enemyVertexArray[index++] = 0.0f;

				enemyVertexArray[index++] = xPos + halfPlayerWidth;
				enemyVertexArray[index++] = yPos + halfPlayerHeight;
				enemyVertexArray[index++] = 1.0f;
				enemyVertexArray[index++] = 1.0f;

				enemyVertexArray[index++] = xPos - halfPlayerWidth;
				enemyVertexArray[index++] = yPos + halfPlayerHeight;
				enemyVertexArray[index++] = 0.0f;
				enemyVertexArray[index++] = 1.0f;

				enemyVertexArray[index++] = xPos + halfPlayerWidth;
				enemyVertexArray[index++] = yPos - halfPlayerHeight;
				enemyVertexArray[index++] = 1.0f;
				enemyVertexArray[index++] = 0.0f;
			}
			halfPlayerHeight *= 2.0f;
			enemyRenderer.setVertices(enemyVertexArray, triangleCount, 12, GL_DYNAMIC_DRAW);
			enemyRenderer.draw(triangleCount);
			delete[] enemyVertexArray;
		
		}
		if (doPhysics) {
			indexXRight = static_cast<int>((playerX - halfPlayerWidth) / blockWidth * -1.0f);
			indexXRightSmall = static_cast<int>((playerX - (halfPlayerWidth * 0.9f)) / blockWidth * -1.0f);
			indexXLeft = static_cast<int>((playerX + halfPlayerWidth) / blockWidth * -1.0f);
			indexXLeftSmall = static_cast<int>((playerX + (halfPlayerWidth * 0.9f)) / blockWidth * -1.0f);
			indexY = static_cast<int>((playerY) / blockHeight * -1.0f);
			indexHeadY = static_cast<int>((playerY + halfPlayerHeight * 0.1f) / blockHeight * -1.0f + 1.0f);
			grounded = false;
			if (collide(tilemap[indexY - 1][indexXRight]) || collide(tilemap[indexY - 1][indexXLeft])) {
				grounded = true;
			}
			if (renderer->getKeyDown(GLFW_KEY_LEFT_SHIFT) && grounded) {
				crouching = true;
			}
			else if (grounded) {
				if (!collide(tilemap[indexY + 1][indexXRightSmall]) && !collide(tilemap[indexY + 1][indexXLeftSmall])) {
					crouching = false;
				}
			}
			playerRenderer.setBool("isCrouching", crouching);

			playerRenderer.draw(2);
			if (renderer->getKeyDown(GLFW_KEY_A)) {
				playerXVel += 1.0f * dt;
			}
			if (renderer->getKeyDown(GLFW_KEY_D)) {
				playerXVel -= 1.0f * dt;
			}
			if (renderer->getKeyDown(GLFW_KEY_SPACE)) {
				jumpKeyCounter += 1;
			}
			else {
				jumpKeyCounter = 0;
			}

			relativeFPS = 1.0f / (dt * 60.0f);
			playerXVel *= (powf(0.8, 1.0f / relativeFPS));
			if (playerXVel > 0.1f) { playerXVel = 0.1f; }
			if (playerXVel < -0.1f) { playerXVel = -0.1f; }
			if (abs(playerXVel) <= 0.001f) { playerXVel = 0.0f; }



			// Modify x position.
			playerX += playerXVel * dt * 5.0f;
			indexXRight = static_cast<int>((playerX - halfPlayerWidth) / blockWidth * -1.0f);
			indexXRightSmall = static_cast<int>((playerX - (halfPlayerWidth * 0.85f)) / blockWidth * -1.0f);
			indexXLeft = static_cast<int>((playerX + halfPlayerWidth) / blockWidth * -1.0f);
			indexXLeftSmall = static_cast<int>((playerX + (halfPlayerWidth * 0.85f)) / blockWidth * -1.0f);
			indexY = static_cast<int>((playerY + halfPlayerHeight * 0.8f) / blockHeight * -1.0f);
			indexHeadY = static_cast<int>((playerY + halfPlayerHeight * 0.1f) / blockHeight * -1.0f + 1.0f);
			indexMiddle = static_cast<int>((playerY - halfPlayerHeight * 0.25f) / blockHeight * -1.0f);

			// When moving right, check for collisions at the right index.
			if (playerXVel < 0.0f) {
				if (collide(tilemap[indexY][indexXRight])) {
					playerX = halfPlayerWidth - indexXRight * blockWidth;
					playerXVel = 0.0f;
				}

			}

			// When moving left, check for collisions at the left index.
			if (playerXVel > 0.0f) {
				if (collide(tilemap[indexY][indexXLeft])) {
					playerX = -indexXLeft * blockWidth - blockWidth - halfPlayerWidth;
					playerXVel = 0.0f;
				}

			}
			bool onLadder = false;
			if (!crouching) {
				// When moving right, check for collisions at the right index.
				if (playerXVel < 0.0f) {
					if (collide(tilemap[indexHeadY][indexXRight])) {
						playerX = halfPlayerWidth - indexXRight * blockWidth;
						playerXVel = 0.0f;
					}

				}

				// When moving left, check for collisions at the left index.
				if (playerXVel > 0.0f) {
					if (collide(tilemap[indexHeadY][indexXLeft])) {
						playerX = -indexXLeft * blockWidth - blockWidth - halfPlayerWidth;
						playerXVel = 0.0f;
					}
				}

				// Check the middle of the player for finer collisions.
				// When moving right, check for collisions at the right index.
				if (playerXVel < 0.0f) {
					if (collide(tilemap[indexMiddle][indexXRight])) {
						playerX = halfPlayerWidth - indexXRight * blockWidth;
						playerXVel = 0.0f;
					}
				}

				// When moving left, check for collisions at the left index.
				if (playerXVel > 0.0f) {
					if (collide(tilemap[indexMiddle][indexXLeft])) {
						playerX = -indexXLeft * blockWidth - blockWidth - halfPlayerWidth;
						playerXVel = 0.0f;
					}
				}
				if (ladder(tilemap[indexMiddle][indexXRightSmall]) || ladder(tilemap[indexMiddle][indexXLeftSmall])) {
					if (renderer->getKeyDown(GLFW_KEY_W)) {
						onLadder = true;
					}
				}
			}
			if (!onLadder) {
				// Move on the Y-axis
				playerYVel += dt * 15.0f;
			}
			else {
				playerYVel = -1.5f;
			}
			playerY += playerYVel * dt * 0.3f;

			// Re-calculate what blocks the player is hitting.
			indexXRight = static_cast<int>((playerX - halfPlayerWidth * 0.99f) / blockWidth * -1.0f);
			indexXRightSmall = static_cast<int>((playerX - (halfPlayerWidth * 0.9f)) / blockWidth * -1.0f);
			indexXLeft = static_cast<int>((playerX + halfPlayerWidth) / blockWidth * -1.0f);
			indexXLeftSmall = static_cast<int>((playerX + (halfPlayerWidth * 0.9f)) / blockWidth * -1.0f);
			indexY = static_cast<int>((playerY) / blockHeight * -1.0f);
			indexHeadY = static_cast<int>((playerY + halfPlayerHeight * 0.1f) / blockHeight * -1.0f + 1.0f);
			indexTop = static_cast<int>((playerY - (blockHeight - halfPlayerHeight) * 2.0f - blockHeight) / blockHeight * -1.0f);

			// If player is travelling down, check if the block in which their feet are is solid.
			if (playerYVel > 0.0f) {
				if (collide(tilemap[indexY - 1][indexXRightSmall]) || collide(tilemap[indexY - 1][indexXLeftSmall])) {
					playerY = -blockHeight * indexY - halfPlayerHeight;
					playerYVel = 0.0f;
				}
			}

			// If the player is travelling up, check if the block above their head is solid.
			if (playerYVel < 0.0f) {
				if (collide(tilemap[indexTop][indexXRightSmall]) || collide(tilemap[indexTop][indexXLeftSmall])) {
					playerY = -blockHeight * (indexTop - 1);
					playerYVel = 0.0f;
				}

			}
			/*
			if (crouching && indexY-3 >= 0 && indexY-3 < tilemap.size() && !grounded && playerYVel > 0.0f) {
				if (!(collide(tilemap[indexY-3][indexXRightSmall]) || collide(tilemap[indexY-3][indexXLeftSmall]))) {
					crouching = false;
				}
			}*/

			// Jump check.
			grounded = false;
			if (collide(tilemap[indexY - 1][indexXRight]) || collide(tilemap[indexY - 1][indexXLeft])) {
				grounded = true;
			}
			if (jumpKeyCounter == 1 && !crouching && grounded) {
				if (!collide(tilemap[indexTop][indexXRightSmall]) && !collide(tilemap[indexTop][indexXLeftSmall])) {
					playerYVel = -7.0f;
				}
			}
			// Draw screen.
			renderer->updateDisplay();
		}
	}
	client.terminate();
	if (doMultiplayer) {
		recvThread.join();
		sendThread.join();
	}
	return 0;
}

int main() {
	Renderer renderer(windowWidth, windowHeight, "The Abyssal Zone");
	//game("CE519869", &renderer, "bob");
	vector<MenuButton> pageButtons;
	pageButtons.push_back(move(MenuButton(0.0f, 0.2f, 0.0f, 0.1f)));
	pageButtons.push_back(move(MenuButton(0.0f, -0.2f, 1.0f, 0.1f)));
	string joinCode;
	vector<Text> pageText;
	string pageID = "home";
	while (true) {
		
		tuple<int, string> data = GUI(&renderer, pageButtons, pageText, pageID);
		int action = get<0>(data);
		string extraData = get<1>(data);
		if (action == 0) {
			game("NONE", &renderer, "NONE");
		}
		if (action == 1) {
			pageButtons = vector<MenuButton>{ MenuButton(0.0f,0.2f, 3.0f, 0.1f),
											 MenuButton(0.0f,-0.2f, 2.0f, 0.1f) };
			pageText = vector<Text>{ Text(0.0f, 0.6f, "BE412450000", 0.1f) } ;
			pageID = "multiplayer";
		}
		if (action == 2) {
			pageButtons = vector<MenuButton>{ MenuButton(0.0f, 0.2f, 0.0f, 0.1f), MenuButton(0.0f, -0.2f, 1.0f, 0.1f) };
			pageText = vector<Text>{};
			pageID = "home";
		}
		if (action == 3 && pageID == "join") {
			cout << "JOINING! USERNAME: [" << extraData << "]" << endl;
			game(joinCode, &renderer, extraData);
		}
		if (action == 3 && pageID == "multiplayer") {
			joinCode = extraData;
			pageButtons = vector<MenuButton>{ MenuButton(0.0f,0.2f, 3.0f, 0.1f), 
				                              MenuButton(0.0f,-0.2f, 2.0f, 0.1f) };
			pageText = vector<Text>{ Text(0.0f, 0.6f, "", 0.1f) };
			pageID = "join";
		}
	}
	return 0;
}