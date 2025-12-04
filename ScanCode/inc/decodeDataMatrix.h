#pragma once
#include <vector>
#include <string>
//gamma:0.0~2.2f
bool decodeDataMatrix(const std::string imagePath, std::string& result, float gamma = 1.0);
bool decodeDataMatrixs(const std::string imagePath, std::vector<std::string>& results, float gamma = 1.0);