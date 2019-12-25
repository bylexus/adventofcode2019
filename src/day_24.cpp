#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cmath>
#include "common.h"

using namespace std;

/**
 * Day 2 - Planet of Discord
 *
 */

typedef vector<unsigned long> tiles_t;

void printTiles(tiles_t &tiles, unsigned int width) {
    for (unsigned int i = 0; i < tiles.size(); ++i)
    {
        if (tiles[i] > 0) {
            cout << '#';
        } else {
            cout << '.';
        }
        if (i % width == width - 1){
            cout << endl;
        }
    }
}

unsigned long calcFieldSum(tiles_t &tiles)
{
    unsigned long sum = 0;
    for (auto t : tiles) {
        sum += t;
    }
    return sum;
}

/**
 * Counts the adjacent nr of bugs to a specific tile
 *
 * @param tiles The tile field vector
 * @param width The field width
 * @param pos The position (index) of the bug to investigate
 */
unsigned int countAdjacentBugs(tiles_t &tiles, unsigned int width, unsigned int pos)
{
    unsigned int sum = 0;
    // to the left:
    if (pos % width > 0 && tiles[pos-1] > 0) sum++;
    // to the top:
    if (pos >= width  && tiles[pos - width] > 0) sum++;
    // to the right:
    if (pos % width < width - 1 && pos + 1 < tiles.size() && tiles[pos+1] > 0) sum++;
    // to the bottom:
    if (pos + width < tiles.size() && tiles[pos + width] > 0) sum++;

    return sum;
}

void solution1(tiles_t &tiles, unsigned int width) {
    set<unsigned long> seenSum;
    tiles_t            nextTiles(tiles);

    unsigned int counter = 0;
    unsigned long fieldSum = 0;
    while (true) {
        // printTiles(tiles, width); cout << endl << endl;
        fieldSum = calcFieldSum(tiles);
        if (seenSum.count(fieldSum) > 0) {
            break;
        }
        seenSum.insert(fieldSum);
        for (unsigned int i = 0; i < tiles.size(); ++i) {
            unsigned int bugcount = countAdjacentBugs(tiles, width,i);
            if (tiles[i] > 0) {
                // seen a bug:
                if (bugcount == 1) {
                    nextTiles[i] = tiles[i];
                } else {
                    nextTiles[i] = 0;
                }
            } else {
                // seen no bug:
                if (bugcount == 1 || bugcount == 2) {
                    nextTiles[i] = pow(2,i);
                } else {
                    nextTiles[i] = 0;
                }
            }
        }
        counter++;
        tiles.clear();
        tiles = nextTiles;
    }


    printTiles(tiles, width); cout << endl << endl;
    cout << "Solution 1: First duplicate biodiversity rating: " << fieldSum << endl;
}

void solution2() {
}

int main(int argc, char *args[])
{
    // bool display = argc >= 3 && string("--display").compare(args[2]) == 0 ? true : false;
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<string> inputLines;
    readLines(args[1],  inputLines);

    tiles_t tiles;
    unsigned int width = 0;

    unsigned int counter = 0;
    for (auto line : inputLines) {
        width = line.length();
        for (auto c : line) {
            if (c == '#') {
                tiles.push_back(pow(2,counter));
            } else {
                tiles.push_back(0UL);
            }
        }
    }

    solution1(tiles, width);

    // solution2(inputLines);
}
