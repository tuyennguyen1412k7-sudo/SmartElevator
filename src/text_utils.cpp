#include "elevator/text_utils.h"

#include <iomanip>
#include <sstream>

#include "elevator/exceptions.h"

bool isBlankChar(char ch) { return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'; }

std::string trim(const std::string& s) {
    std::size_t first = 0;
    while (first < s.size() && isBlankChar(s[first])) ++first;

    std::size_t last = s.size();
    while (last > first && isBlankChar(s[last - 1])) --last;

    std::string result;
    for (std::size_t i = first; i < last; ++i) result += s[i];
    return result;
}

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> parts;
    std::istringstream stream{s};
    std::string token;
    while (std::getline(stream, token, sep)) parts.push_back(trim(token));
    return parts;
}

std::string toLower(const std::string& s) {
    std::string result = s;
    for (char& ch : result)
        if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    return result;
}

std::string formatFixed(double value, int digits) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(digits) << value;
    return os.str();
}

std::string padRight(const std::string& s, std::size_t width) {
    std::string result = s;
    while (result.size() < width) result += ' ';
    return result;
}

std::string padLeft(const std::string& s, std::size_t width, char fill) {
    std::string result = s;
    while (result.size() < width) result = fill + result;
    return result;
}

int parseInt(const std::string& s, const std::string& what) {
    std::istringstream in{s};
    int value{};
    char extra{};
    if (!(in >> value) || (in >> extra))
        throw ValidationError(what + " phải là số nguyên, nhận được '" + s + "'");
    return value;
}

double parseDouble(const std::string& s, const std::string& what) {
    std::istringstream in{s};
    double value{};
    char extra{};
    if (!(in >> value) || (in >> extra))
        throw ValidationError(what + " phải là số, nhận được '" + s + "'");
    return value;
}
