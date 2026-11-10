#pragma once
#include <string>
#include <vector>

std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s, char sep);
std::string toLower(const std::string& s);

std::string formatFixed(double value, int digits);
std::string padRight(const std::string& s, std::size_t width);
std::string padLeft(const std::string& s, std::size_t width, char fill = ' ');

int parseInt(const std::string& s, const std::string& what);
double parseDouble(const std::string& s, const std::string& what);
