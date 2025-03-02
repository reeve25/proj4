#include "../include/StringUtils.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>


namespace StringUtils{

std::string Slice(const std::string &str, ssize_t start, ssize_t end) noexcept{
    if (start < 0) {
        start = str.length() + start;
    }
    if (end < 0) {
        end = str.length() + end;
    }
    return str.substr(start, end - start);
}

std::string Capitalize(const std::string &str) noexcept{
    if (str.length() == 0) {
        return str;
    }
    std::string str2 = "";
    str2 += Upper(str.substr(0, 1));
    str2 += Lower(str.substr(1, str.length()-1));
    return str2;
    /*std::string str2 = str;
    if (str.length() > 0){
        std::transform(str2.begin(), str2.begin() + 1, str2.begin(),::toupper);
        return str2;
    }
    return str;*/
}

std::string Upper(const std::string &str) noexcept{
    std::string str2 = str;
    for (std::size_t i = 0; i < str.length(); i++) {
        str2[i] = toupper(str[i]);
    }
    return str2;
}

std::string Lower(const std::string &str) noexcept{
    std::string str2 = str;
    for (std::size_t i = 0; i < str.length(); i++) {
        str2[i] = tolower(str2[i]);
    }
    return str2;
}

std::string LStrip(const std::string &str) noexcept{
    std::size_t i = 0;
    int count = 0;
    while (i < str.length() && str[i] == ' ') {
        count++;
        i++;
    }
    return str.substr(count, str.length() - count);
}

std::string RStrip(const std::string &str) noexcept{
    std::size_t i = str.length() - 1;
    int count = 0;
    while (i != std::string::npos && str[i] == ' ') {
        count++;
        i--;
    }
    return str.substr(0, str.length() - count);
}

std::string Strip(const std::string &str) noexcept{
    return RStrip(LStrip(str));
}

std::string Center(const std::string &str, int width, char fill) noexcept{
    int count = width - str.length();
    if (count < 0) {
        return str;
    }
    std::string left(count / 2, fill);
    std::string right(count - count / 2, fill);
    return left + str + right;
}

std::string RJust(const std::string &str, int width, char fill) noexcept{
    int count = width - str.length();
    if (count < 0) {
        return str;
    }
    std::string left(count, fill);
    return left + str;
}

std::string LJust(const std::string &str, int width, char fill) noexcept{
    int count = width - str.length();
    if (count < 0) {
        return str;
    }
    std::string right(count, fill);
    return str + right;
}

std::string Replace(const std::string &str, const std::string &old, const std::string &rep) noexcept{
    if (old.length() == 0) return str;
    int count = str.length() - old.length();
    if (count < 0) {
        return str;
    }
    std::string str2 = str;
    std::size_t i = str2.find(old);
    while (i != std::string::npos) {
        str2.replace(i, old.length(), rep);
        i += rep.length();
        i = str2.find(old, i);
    }
    return str2;
}

std::vector< std::string > Split(const std::string &str, const std::string &splt) noexcept{
    std::vector<std::string> arr = {};
    if (splt.length() == 0) {
        if (str.length() == 0) {
            arr.push_back("");
        } else {
            for (char c: str) {
                arr.push_back(std::string(1, c));
            }
        }
    }
    else if (str.length() == 0) {
        arr.push_back("");
    } else {
        int start = 0;
        std::size_t i = str.find(splt);
        while (i != std::string::npos) {
            arr.push_back(str.substr(start, i - start));
            start = i + splt.length();
            i = str.find(splt, start);
        }
        arr.push_back(str.substr(start));
    }
    return arr;
}

std::string Join(const std::string &str, const std::vector< std::string > &vect) noexcept{
    if (vect.size() == 0) {
        return "";
    }
    std::string output = vect[0];
    for (std::size_t i = 1; i < vect.size(); i++) {
        output += str + vect[i];
    }
    return output;
}

std::string ExpandTabs(const std::string &str, int tabsize) noexcept{
    if (tabsize <= 0) {
        std::string output;
        for (char c : str) {
            if (c != '\t') {
                output += c;
            }
        }
        return output;
    }
    std::string output = "";
    int currentColumn = 0;
    for (char c: str) {
        if (c == '\t') {
            int spacesToAdd = tabsize - (currentColumn % tabsize);
            output.append(spacesToAdd, ' ');
            currentColumn += spacesToAdd;
        } else {
            output += c;
            currentColumn++;
            if (c == '\n') {
                currentColumn = 0;
            }
        }
    }
    return output;
}

int EditDistance(const std::string &left, const std::string &right, bool ignorecase) noexcept {
    std::string newLeft = left;
    std::string newRight = right;
    if (ignorecase) {
        newLeft = Lower(left);
        newRight = Lower(right);
    }
    std::size_t m = left.length();
    std::size_t n = right.length();

    std::vector<std::vector<int>> mat(m+1, std::vector<int>(n+1, 0));

    for (std::size_t i = 0; i <= m; i++) {
        mat[i][0] = i;
    }
    for (std::size_t j = 0; j <= n; j++) {
        mat[0][j] = j;
    }

    for (std::size_t i = 1; i <= m; i++) {
        for (std::size_t j = 1; j <= n; j++) {

            int subCost;
            if (newLeft[i-1] == newRight[j-1]) {
                subCost = 0;
            } else {
                subCost = 1;
            }

            mat[i][j] = std::min({mat[i-1][j] + 1, mat[i][j-1] + 1, mat[i-1][j-1] + subCost});
        }
    }

    return mat[m][n];
}

};