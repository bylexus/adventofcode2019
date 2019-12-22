#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include "common.h"
#include "intcode.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <thread>
#include <chrono>

using namespace std;
using namespace boost::multiprecision;
using namespace boost;

/**
 * Day 19 - Tractor Beam
 *
 * This one is a bit of detective work and a bisect to find the right y position.
 * Detective work because I first had to get an idea how steep the beam is to narrow the start x for
 * each y.
 */

class Point
{
public:
    Coord coord{0, 0};
    char value = 0;
    bool visited = false;

    Point(int x, int y, char v) : value{v}
    {
        coord = Coord(x, y);
    }
    Point()
    {
    }
};

typedef vector<Point> map_line_t;
typedef vector<map_line_t> map_t;
map_t tractorMap;

/**
 * Prints the map in its current state as ASCII Art
 */
void printMap()
{
    for (auto l : tractorMap)
    {
        for (auto c : l)
        {
            cout << (c.value == 0 ? '.' : (c.value == 1 ? '#' : '?'));
        }
        cout << endl;
    }
}

/**
 * Starts solution 1
 */
int solution1(memory_t &data)
{
    map_line_t line;
    int counter = 0;
    int maxX = 50, maxY = 50;
    for (int y = 0; y < maxY; y++)
    {
        line.clear();
        for (int x = 0; x < maxX; x++)
        {
            Program program(&data);
            program.inputValues.push(cpp_int(x));
            program.inputValues.push(cpp_int(y));
            program.runProgram();
            char output = static_cast<char>(program.output);
            line.push_back(Point(x, y, output));
            if (output == 1)
            {
                counter++;
            }
        }
        tractorMap.push_back(line);
    }
    return counter;
}

char calcBeamPos(int x, int y, memory_t &data)
{
    Program program(&data);
    program.inputValues.push(cpp_int(x));
    program.inputValues.push(cpp_int(y));
    program.runProgram();
    char output = static_cast<char>(program.output);
    return output;
}

/**
 * Starts solution 2
 *
 * Solution 1 shows that y / x varies between 0.85 and 0.95 (tends to become 0.91).
 * So I will try to find the fitting y position with a bisect algorithm:
 * For an y, which represents the lower-left corner of the ship, I calc if the ship fits in the beam by
 *   testint the calculated X values of lower-left x and upper-right x.
 *
 * - Start at an arbitary y. Find the first x pos where x = '#' by starting at a lower bound x, then increase by one
 * - calc the upper-right x of the ship, and check if it fits EXACTLY in.
 * - If upper right x is on empty space, increase y (bisect)
 * - if upper right x is in '#', and right of it is '#' too, decrease y (bisect)
 * - if upper right x is in '#', and right of it is ' ', we found it!
 */
int solution2(memory_t &data)
{
    double lowerProportion = 0.85;
    double upperProportion = 0.95;
    int shipWidth = 100;
    int minY = 10, maxY = 10000, actY = 0, minX = 0, maxX = 0;
    while (minY < maxY)
    {
        actY = (maxY + minY) / 2;
        minX = floor(actY / upperProportion);
        maxX = ceil(actY / lowerProportion);
        while (true)
        {
            // Step 1: Find lower y/x, where x is the first beam position
            char leftOutput = calcBeamPos(minX, actY, data);
            char rightOutput = calcBeamPos(minX + 1, actY, data);
            if (leftOutput == 1)
            {
                // Not good, should not happen:
                cerr << "Lower x bound too small, exit" << endl;
                exit(1);
            }
            if (leftOutput == 0 && rightOutput == 1)
            {
                // Found 1st x
                // cout << "Found first x at: " << minX + 1 << " Y: " << actY << endl;
                minX++;
                break;
            }
            else
            {
                minX++;
            }
        }
        // Found min (left) x, check if ship fits into beam:
        maxX = minX + shipWidth; // should be empty space
        char leftOutput = calcBeamPos(maxX - 1, actY - shipWidth + 1, data);
        char rightOutput = calcBeamPos(maxX, actY - shipWidth + 1, data);
        if (leftOutput == 1 && rightOutput == 0)
        {
            // Ship fits perfectly, done!
            minY = (actY - shipWidth + 1);
            cout << "Ship fits at: " << minX << ":" << minY  << endl;
            break;
        }
        else if (leftOutput == 1 && rightOutput == 1)
        {
            // beam already too broad, make Y smaller
            maxY = actY;
        }
        else
        {
            // Beam too small, increase y
            minY = actY;
        }
    }
    return 10000 * minX + minY;
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
    memory_t data;
    readIntcodeProgramFromFile(args[1], data);

    int nrOfBeamPos = solution1(data);
    printMap();
    cout << "Solution 1: Nr of Tractor pos: " << nrOfBeamPos << endl;
    int coordProd = solution2(data);
    cout << "Solution 2: Ship Fit Coord Product: " << coordProd << endl;

}
