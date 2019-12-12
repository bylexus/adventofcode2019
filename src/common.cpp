#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "common.h"

using namespace std;

/**
 * Day 2 - Program Alarm
 */


/**
 * Read data from file - split by line/delimiter
 *
 * Fills a vector with all input data
 */
template<typename T>
void readData(char *filename, char delim, vector<T> &data)
{
    ifstream infile(filename);
    string token;
    T nr;
    while(getline(infile, token, delim)) {
        istringstream(token) >> nr;
        data.push_back(nr);
    }
}

// define template functions as needed:
template void readData<long>(char *, char, vector<long> &);
template void readData<int>(char *, char, vector<int> &);
template void readData<string>(char *, char, vector<string> &);

/**
 * Reads a file and returns the single lines in a vector
 */
void readLines(char *filename, vector<string> &data) {
    ifstream infile(filename);
    string token;
    while(getline(infile, token)) {
        data.push_back(token);
    }
}

void split(const string &str, char delim, vector<string> &data) {
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delim)) {
        data.push_back(token);
    }
}
