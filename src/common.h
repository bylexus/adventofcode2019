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

class Coord {
    public:
        int x,y;

        Coord():Coord(0,0) {}
        Coord(int x, int y): x{x}, y{y} {}

        bool operator<(const Coord &p) const {
            return x < p.x || y < p.y;
        }
};
#endif
