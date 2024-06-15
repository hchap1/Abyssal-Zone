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
    vector<float> playerFrames;
    vector<float> playerDirections;
    vector<string> playerIDs;
    vector<float> enemyXPositions;
    vector<float> enemyYPositions;
    vector<string> enemyNames;

    string encoded;
    int playerCount = 0;
    int enemyCount = 0;
	Packet(string encodedString) {
        encoded = encodedString;
        vector<string> packetData = splitString(encodedString, '|');
        size_t delimCount = count(encodedString.begin(), encodedString.end(), '|');
        if (packetData.size() > 1) {
            string enemyData = packetData[0];
            string playerData = packetData[1];
            vector<string> data;

            vector<string> enemies = splitString(enemyData, '/');
            enemyCount = 0;
            for (string enemy : enemies) {
                enemyCount += 1;
                data = splitString(enemy, ',');
                if (data.size() >= 3) {
                    enemyXPositions.push_back(stof(data[0]));
                    enemyYPositions.push_back(stof(data[1]));
                    enemyNames.push_back(data[2]);
                }
            }

            vector<string> players = splitString(playerData, '/');
            playerCount = 0;
            for (string player : players) {
                playerCount += 1;
                data = splitString(player, ',');
                if (data.size() >= 6) {
                    playerXPositions.push_back(stof(data[0]));
                    playerYPositions.push_back(stof(data[1]));
                    playerFrames.push_back(stof(data[3]));
                    playerDirections.push_back(stof(data[4]));
                    playerIDs.push_back(splitString(data[5], '!')[0]);
                    if (data[2] == "true" || data[2] == "1") { 
                        playerCrouchingBools.push_back(true); 
                    }
                    else { playerCrouchingBools.push_back(false); }
                }
            }
        }
	}
private:
    
};