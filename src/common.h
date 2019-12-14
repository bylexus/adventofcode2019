#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>

using namespace std;

/**
 * Read data from file - split by line/delimiter
 *
 * Fills a vector with all input data
 */
template<typename T>
void readData(char *filename, char delim, vector<T> &data);

void readLines(char *filename, vector<string> &data);

void split(const string &str, char delim, vector<string> &data);

string trim(const string &str);
#endif
