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
    vector<string> playerIDs;

    string encoded;
    int playerCount = 0;
	Packet(string encodedString) {
        // Expected: ENEMYDATA|PLAYERDATA
        //  WHERE: ENEMYDATA  = x,y,z/x,y,z/x,y,z
        //         PLAYERDATA = x,y,z/x,y,z/x,y,z
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
                if (data.size() > 4) {
                    playerXPositions.push_back(stof(data[0]));
                    playerYPositions.push_back(stof(data[1]));
                    playerFrames.push_back(stof(data[3]));
                    playerIDs.push_back(splitString(data[4], '!')[0]);
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