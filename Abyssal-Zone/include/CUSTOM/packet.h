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
    vector<float> playerXPositions;
    vector<float> playerYPositions;
    vector<bool> playerCrouchingBools;
    string encoded;
    int playerCount = 0;
	Packet(string encodedString) {
        encoded = encodedString;
        vector<string> packetData = splitString(encodedString, '|');
        size_t delimCount = count(encodedString.begin(), encodedString.end(), '|');
        if (packetData.size() > 1) {
            string enemyData = packetData[0];
            string playerData = packetData[1];
            vector<string> enemies = splitString(enemyData, '/');
            vector<string> players = splitString(playerData, '/');
            vector<string> data;
            playerCount = 0;
            for (string player : players) {
                playerCount += 1;
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
    tuple<float*, int> constructPlayerVertices(float hpw, float hph) {
        size_t size = playerCrouchingBools.size();
        const size_t vertexDataSize = size * 30;
        float* multiplayerVertexArray = new float[vertexDataSize];
        int triangleCount = 0;
        size_t index = 0;
        for (int i = 0; i < playerCrouchingBools.size(); i++) {
            float xPos = playerXPositions[i];
            float yPos = playerYPositions[i];
            bool isCrouching = playerCrouchingBools[i];
            float crouching = 0.0f;
            if (isCrouching) { crouching = 1.0f; }
            triangleCount += 2;
            multiplayerVertexArray[index++] = xPos - hpw;
            multiplayerVertexArray[index++] = yPos + hph;
            multiplayerVertexArray[index++] = 0.0f;
            multiplayerVertexArray[index++] = 0.5f;
            multiplayerVertexArray[index++] = crouching;

            multiplayerVertexArray[index++] = xPos - hpw;
            multiplayerVertexArray[index++] = yPos + hph;
            multiplayerVertexArray[index++] = 0.0f;
            multiplayerVertexArray[index++] = 1.0f;
            multiplayerVertexArray[index++] = crouching;

            multiplayerVertexArray[index++] = xPos + hpw;
            multiplayerVertexArray[index++] = yPos - hph;
            multiplayerVertexArray[index++] = 1.0f;
            multiplayerVertexArray[index++] = 0.5f;
            multiplayerVertexArray[index++] = crouching;

            multiplayerVertexArray[index++] = xPos + hpw;
            multiplayerVertexArray[index++] = yPos + hph;
            multiplayerVertexArray[index++] = 1.0f;
            multiplayerVertexArray[index++] = 1.0f;
            multiplayerVertexArray[index++] = crouching;

            multiplayerVertexArray[index++] = xPos - hpw;
            multiplayerVertexArray[index++] = yPos + hph;
            multiplayerVertexArray[index++] = 0.0f;
            multiplayerVertexArray[index++] = 1.0f;
            multiplayerVertexArray[index++] = crouching;

            multiplayerVertexArray[index++] = xPos + hpw;
            multiplayerVertexArray[index++] = yPos - hph;
            multiplayerVertexArray[index++] = 1.0f;
            multiplayerVertexArray[index++] = 0.5f;
            multiplayerVertexArray[index++] = crouching;
        }
        return make_tuple(multiplayerVertexArray, triangleCount);
	}
private:
    
};