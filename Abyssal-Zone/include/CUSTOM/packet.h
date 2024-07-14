#pragma once
#include <string>
#include <vector>
#include "CUSTOM/renderer.h"
#include <algorithm>


std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

class Packet {
public:
    std::vector<float> playerXPositions;
    std::vector<float> playerYPositions;
    std::vector<bool> playerCrouchingBools;
    std::vector<float> playerFrames;
    std::vector<float> playerDirections;
    std::vector<std::string> playerIDs;
    std::vector<float> enemyXPositions;
    std::vector<float> enemyYPositions;
    std::vector<std::string> enemyNames;

    std::string encoded;
    int playerCount = 0;
    int enemyCount = 0;
	Packet(std::string encodedString) {
        encoded = encodedString;
        std::vector<std::string> packetData = splitString(encodedString, '|');
        size_t delimCount = std::count(encodedString.begin(), encodedString.end(), '|');
        if (packetData.size() > 1) {
            std::string enemyData = packetData[0];
            std::string playerData = packetData[1];
            std::vector<std::string> data;

            std::vector<std::string> enemies = splitString(enemyData, '/');
            enemyCount = 0;
            for (std::string enemy : enemies) {
                enemyCount += 1;
                data = splitString(enemy, ',');
                if (data.size() >= 3) {
                    enemyXPositions.push_back(std::stof(data[0]));
                    enemyYPositions.push_back(std::stof(data[1]));
                    enemyNames.push_back(data[2]);
                }
            }

            std::vector<std::string> players = splitString(playerData, '/');
            playerCount = 0;
            for (std::string player : players) {
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