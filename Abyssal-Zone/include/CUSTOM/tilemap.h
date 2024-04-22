#pragma once
#include <vector>

std::tuple<std::vector<std::vector<int>>, std::vector<std::vector<int>>> loadTilemap(int levelID);
int* loadLevelData(int levelID);