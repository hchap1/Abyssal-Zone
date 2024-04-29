#pragma once
#include <string>
#include <vector>
#include "CUSTOM/renderer.h"
using namespace std;

vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

class Packet {
public:
	Packet(string encodedString) {
        encoded = encodedString;
        vector<string> packetData = splitString(encodedString, '|');
        string enemyData = packetData[0];
        string playerData = packetData[1];
        vector<string> enemies = splitString(enemyData, '/');
        vector<string> players = splitString(playerData, '/');
        vector<string> data;
        for (string player : players) {
            data = splitString(player, ',');
            playerXPositions.push_back(stof(data[0]));
            playerYPositions.push_back(stof(data[1]));
            if (data[2] == "true") { playerCrouchingBools.push_back(true); }
            else { playerCrouchingBools.push_back(false); }
        }
	}
	Packet(vector<float> xPositions, vector<float> yPositions, vector<bool> crouchingBools) {
        
	}
    void constructPlayerVertices(RenderLayer* playerRenderer, float hpw, float hph) {
        const size_t vertexDataSize = sizeof(playerCrouchingBools) * 30;
        float playerVertexData[vertexDataSize];
        int triangleCount = 0;
        for (int index = 0; index < sizeof(playerCrouchingBools); index++) {
            float xPos = playerXPositions[index];
            float yPos = playerYPositions[index];
            bool isCrouching = playerCrouchingBools[index];
            float crouching = 0.0f;
            if (isCrouching) { crouching = 1.0f; }
            triangleCount += 2;
            playerVertexData[index++] = -hpw;
            playerVertexData[index++] = -hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = -hpw;
            playerVertexData[index++] = hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = hpw;
            playerVertexData[index++] = -hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = hpw;
            playerVertexData[index++] = hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = -hpw;
            playerVertexData[index++] = hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = hpw;
            playerVertexData[index++] = -hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;
        }
        playerRenderer->setVertices(playerVertexData, triangleCount, 30, GL_DYNAMIC_DRAW);
	}
private:
    vector<float> playerXPositions;
    vector<float> playerYPositions;
    vector<float> playerCrouchingBools;
    string encoded;
};