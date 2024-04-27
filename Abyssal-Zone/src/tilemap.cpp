#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <CUSTOM\tilemap.h>
#include <algorithm>
using namespace std;

vector<vector<int>> loadTilemap(int levelID) {
    const string filename = "assets/levels/" + to_string(levelID) + ".tilemap";
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
    }

    vector<vector<int>> tilemap;

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int number;
        vector<int> row;
        while (iss >> number) {
            if (number > 7) {
                number += 7;
            }
            row.push_back(number);
        }

        // Append row
        tilemap.push_back(row);
    }

    file.close();

    reverse(tilemap.begin(), tilemap.end());
    return tilemap;
}

int* loadLevelData(int levelID) {
    int data[4] = {};
    ifstream dataFile("assets/levels/" + to_string(levelID) + ".tilemap");
    dataFile >> data[0] >> data[1] >> data[2] >> data[3];
    dataFile.close();
    return data;
}
