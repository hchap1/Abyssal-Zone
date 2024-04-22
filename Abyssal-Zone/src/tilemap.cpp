#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <CUSTOM\tilemap.h>
#include <algorithm>
using namespace std;

tuple<vector<vector<int>>, vector<vector<int>>> loadTilemap(int levelID) {
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
            row.push_back(number);
            cout << number;
        }

        cout << endl;

        // Append row
        tilemap.push_back(row);
    }

    file.close();

    reverse(tilemap.begin(), tilemap.end());

    const string bgfilename = "assets/levels/" + to_string(levelID) + ".background";
    ifstream bgfile(bgfilename);
    if (!bgfile.is_open()) {
        cerr << "Error opening file: " << bgfilename << endl;
    }

    vector<vector<int>> bgTilemap;

    string bgline;
    while (getline(bgfile, bgline)) {
        istringstream bgiss(bgline);
        int bgnumber;
        vector<int> bgrow;
        while (bgiss >> bgnumber) {
            bgrow.push_back(bgnumber);
            cout << bgnumber;
        }

        cout << endl;

        // Append row
        bgTilemap.push_back(bgrow);
    }

    bgfile.close();

    reverse(bgTilemap.begin(), bgTilemap.end());
    return make_tuple(tilemap, bgTilemap);
}

int* loadLevelData(int levelID) {
    int data[4] = {};
    ifstream dataFile("assets/levels/" + to_string(levelID) + ".tilemap");
    dataFile >> data[0] >> data[1] >> data[2] >> data[3];
    dataFile.close();
    return data;
}
