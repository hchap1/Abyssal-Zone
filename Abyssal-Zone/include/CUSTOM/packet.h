#pragma once
#include <string>
#include <vector>
#include "CUSTOM/renderer.h"
#include <algorithm>
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
        cout << "PACKET: " << encodedString << endl;
        encoded = encodedString;
        vector<string> packetData = splitString(encodedString, '|');
        cout << "STRING SPLIT: SIZE: " << packetData.size() << endl;
        size_t delimCount = count(encodedString.begin(), encodedString.end(), '|');
        if (packetData.size() > 1) {
            string enemyData = packetData[0];
            cout << "PACKET ACCEPTED." << endl;
            string playerData = packetData[1];
            vector<string> enemies = splitString(enemyData, '/');
            vector<string> players = splitString(playerData, '/');
            vector<string> data;
            cout << "PLAYERCOUNT: " << players.size() << endl;
            for (string player : players) {
                data = splitString(player, ',');
                if (data.size() > 2) {
                    playerXPositions.push_back(stof(data[0]));
                    playerYPositions.push_back(stof(data[1]));
                    if (data[2] == "true") { playerCrouchingBools.push_back(true); }
                    else { playerCrouchingBools.push_back(false); }
                }
            }
        }
	}
	Packet(vector<float> xPositions, vector<float> yPositions, vector<bool> crouchingBools) {
        playerXPositions = xPositions;
        playerYPositions = yPositions;
        playerCrouchingBools = crouchingBools;
        string playerString = "";
        for (int i = 0; i < sizeof(crouchingBools); i++) {
            playerString += to_string(xPositions[i]);
            playerString += ",";
            playerString += to_string(yPositions[i]);
            playerString += ",";
            playerString += to_string(crouchingBools[i]);
            playerString += "/";
        }
        playerString.replace(playerString.length(), 1, "");
        encoded = "a,b,c/d,e,f|" + playerString;
	}
    int constructPlayerVertices(RenderLayer* playerRenderer, float hpw, float hph) {
        const size_t vertexDataSize = sizeof(playerCrouchingBools) * 30;
        float playerVertexData[vertexDataSize];
        int triangleCount = 0;
        for (int index = 0; index < playerCrouchingBools.size(); index++) {
            float xPos = playerXPositions[index];
            float yPos = playerYPositions[index];
            bool isCrouching = playerCrouchingBools[index];
            float crouching = 0.0f;
            if (isCrouching) { crouching = 1.0f; }
            triangleCount += 2;
            playerVertexData[index++] = xPos - hpw;
            playerVertexData[index++] = yPos + hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = xPos - hpw;
            playerVertexData[index++] = yPos + hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = xPos + hpw;
            playerVertexData[index++] = yPos - hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = xPos + hpw;
            playerVertexData[index++] = yPos + hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = xPos - hpw;
            playerVertexData[index++] = yPos + hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = xPos + hpw;
            playerVertexData[index++] = yPos - hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;
        }
        playerRenderer->setVertices(playerVertexData, triangleCount, 30, GL_DYNAMIC_DRAW);
        return triangleCount;
	}
private:
    vector<float> playerXPositions;
    vector<float> playerYPositions;
    vector<bool> playerCrouchingBools;
    string encoded;
};