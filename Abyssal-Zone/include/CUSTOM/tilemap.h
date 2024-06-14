#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <tuple>
#include "CUSTOM/packet.h"
using namespace std;

tuple<vector<vector<int>>, float(*)[4], int> loadTilemapFromFile(int levelID) {
    const string filename = "assets/levels/" + to_string(levelID) + ".tilemap";
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
    }

    vector<vector<int>> tilemap;
    static float lightArray[64][4] = {};
    string line;
    int yCount = 0;
    int xCount = 0;
    int index = 0;
    int number;
    while (getline(file, line)) {
        istringstream iss(line);
        vector<int> row;
        while (iss >> number) {
            if (number > 7) {
                number += 7;
            }
            if ((number == 3 || number == 7) && index < 64) {
                // x, y, number, brightness
                lightArray[index][0] = static_cast<float>(xCount);
                lightArray[index][1] = static_cast<float>(yCount);
                lightArray[index][2] = static_cast<float>(number);
                lightArray[index][3] = 1.0f;
                index++;
            }
            row.push_back(number);
            xCount++;
        }
        yCount++;
        xCount = 0;
        // Append row
        tilemap.push_back(row);
    }
    for (int i = 0; i < index; i++) {
        lightArray[i][1] = tilemap.size() - 1 - lightArray[i][1];
    }
    file.close();

    reverse(tilemap.begin(), tilemap.end());
    return make_tuple(tilemap, lightArray, index);
}

int* loadLevelData(int levelID) {
    int data[4] = {};
    ifstream dataFile("assets/levels/" + to_string(levelID) + ".tilemap");
    dataFile >> data[0] >> data[1] >> data[2] >> data[3];
    dataFile.close();
    return data;
}

tuple<vector<vector<int>>, float(*)[4], int> loadTilemapFromString(string message) {
    static float lightArray[64][4] = {};
    vector<string> data = splitString(message, '|');
    string tilemapString = splitString(data[1], '!')[0];
    vector<vector<int>> newTilemap;
    int xCount = 0;
    int yCount = 0;
    int index = 0;
    int size = count(tilemapString.begin(), tilemapString.end(), '/') + 1;
    for (string row : splitString(tilemapString, '/')) {
        vector<int> v_row;
        for (string tile : splitString(row, ',')) {
            int t = stoi(tile);
            if (t > 7) {
                t += 7;
            }
            v_row.push_back(t);
            if (t > 7) {
                t += 7;
            }
            if ((t == 3 || t == 7) && index < 64) {
                // x, y, number, brightness
                lightArray[index][0] = static_cast<float>(xCount);
                lightArray[index][1] = static_cast<float>(yCount);
                lightArray[index][2] = static_cast<float>(t);
                lightArray[index][3] = 1.0f;
                index++;
            }
            xCount++;
        }
        yCount++;
        xCount = 0;
        newTilemap.push_back(v_row);
    }
    return make_tuple(newTilemap, lightArray, index);
}