#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include "common.h"
#include "intcode.h"
#include <thread>
#include <chrono>

using namespace std;

/**
 * Day 15 - Oxygen System
 *
 * Classical backtrack problem - at least for part 1.
 */

class Point
{
public:
    int x = 0;
    int y = 0;
    char object = -1;
};

map<string, Point *> coords;
vector<vector<Point *>> floorMap;


/**
 * Returns true if the position was new, so not yet visited,
 * and stores the coordinate as new Point object with its original x/y values,
 * and an object char (e.g. ' ' for floor, 'O' for oxygen ...)
 *
 * true means: new position, not yet visited
 * false means: already seen, no action taken
 */
bool markPosition(int x, int y, char object)
{
    string key = to_string(x) + ":" + to_string(y);
    if (coords.count(key) == 0)
    {
        Point *p = new Point();
        p->x = x;
        p->y = y;
        p->object = object;
        coords[key] = p;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * returns true if the given position is not in the coords map: not visited yet,
 * or false if the coordinate was already stored / visited.
 */
bool checkPosition(int x, int y)
{
    string key = to_string(x) + ":" + to_string(y);
    return coords.count(key) == 0;
}

/**
 * The workhorse for Solution 1: The backtrack mapping alrogithm:
 *
 * *** Assumption ***: This is a straight forward maze, with no loops and only one
 * way to the exit .... may be this is a FALSE assumption, I don't know yet...
 * we are at starX, starty.
 * Main idea is to:
 * - walk in each direction:
 * - if a wall is hit, return "wall"
 * - if we can walk, recurse:
 *   - if recursion was successful, found the target, count steps
 *     In this case, backtrack again, to find a (possibly) shorter way.
 *   - if recursion hit a wall, move robot back to its startX, startY, backtrack
 */
cpp_int walk(Program *p, int startX, int startY, int moveCounter)
{
    int origX = startX;
    int origY = startY;

    vector<int> dirs{1, 4, 2, 3}; // north, east, south, west
    markPosition(startX, startY, ' ');

    for (int i = 0; i < dirs.size(); i++)
    {
        int dir = dirs[i];
        startX = origX;
        startY = origY;

        // Try to walk in a direction:
        switch (dir)
        {
        case 1:
            startY--;
            break;
        case 2:
            startY++;
            break;
        case 3:
            startX--;
            break;
        case 4:
            startX++;
            break;
        }

        // Check if we have visited the next position already: If so, do not consider
        // it anymore, continue in the next direction:
        if (!checkPosition(startX, startY))
        {
            continue;
        }

        // Move the robot in the actual direction:
        p->inputValues.push(cpp_int(dir));
        p->runProgram();
        cpp_int output = p->output;

        // Hit a wall: continue to other direction
        if (output == 0)
        {
            markPosition(startX, startY, '#');
            continue;
        }

        // Found oxygen system: Output movement counter, as this is solution 1,
        // but continue mapping, as the whole map is needed for solutoin 2:
        if (output == 2)
        {
            markPosition(startX, startY, 'O');
            moveCounter++;
            cout << "Solution 1: Oxygen system found after " << moveCounter << " steps" << endl;
            return output;
        }

        // The robot is moved to an empty, unvisited floor:
        if (output == 1)
        {
            // Look forward by recursively start the walk() method again:
            // It can only respond with 0 (hit wall somewhere beyond)
            // or 2 (found oxygen source). In any case, step back to where the robot came,
            // to map the other directions.
            cpp_int result{0};
            result = walk(p, startX, startY, moveCounter + 1);

            // we walked, but hit a wall. Not good. Go back.
            if (result == 0)
            {
                p->inputValues.push(cpp_int(dirs[(i + 2) % 4]));
                p->runProgram();
                continue;
            }

            // good, found the oxygen system somewhere ahead. Move on, to map the location
            if (result == 2)
            {
                p->inputValues.push(cpp_int(dirs[(i + 2) % 4]));
                p->runProgram();
                continue;
            }
        }
    }
    // If we reach this far, we could not found any direction to walk to...
    // We return with 'wall hit', as this is a dead end.
    return cpp_int(0);
}

/**
 * The backtrack function walk() has built a coordinate map
 * with relative (to the start, which is 0/0) coordinates and unknown map size.
 * Here we built a Y x X vector with normalized coordinates: Each entry contains
 * a Point with its normalized coordinates and what it contains:
 *
 * object: ' ': empty floor
 * object: '#': wall
 * object: 'O': Oxygen source
 *
 * Returns the Point to the found Oxygen source
 */
Point *buildFloorMap()
{
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    Point *oxygenPos;
    for (map<string, Point *>::iterator it = coords.begin(); it != coords.end(); ++it)
    {
        Point *p = it->second;
        if (minX > p->x)
        {
            minX = p->x;
        }
        if (minY > p->y)
        {
            minY = p->y;
        }
        if (maxX < p->x)
        {
            maxX = p->x;
        }
        if (maxY < p->y)
        {
            maxY = p->y;
        }
    }
    for (int y = minY; y <= maxY; ++y)
    {
        vector<Point *> line;

        for (int x = minX; x <= maxX; ++x)
        {
            string key = to_string(x) + ":" + to_string(y);
            Point *p;
            if (coords.count(key) > 0)
            {
                p = coords[key];
                if (p->object == 'O')
                {
                    oxygenPos = p;
                }
            }
            else
            {
                // Unknown location, not visited, must be unreachable wall
                p = new Point();
                p->object = '#';
            }
            p->x = x - minX;
            p->y = y - minY;
            line.push_back(p);
        }
        floorMap.push_back(line);
    }
    return oxygenPos;
}


/**
 * Prints the map in its current state as ASCII Art
 */
void printMap()
{
    for (vector<vector<Point *>>::size_type y = 0; y < floorMap.size(); ++y)
    {
        for (vector<Point *>::size_type x = 0; x < floorMap[y].size(); ++x)
        {
            Point *p = floorMap[y][x];
            char block = p->object;
            switch (block)
            {
            case ' ':
                cout << "░";
                break;
            case '#':
                cout << "▇";
                break;
            case 'O':
                cout << "●";
                break;
            default:
                cout << block;
            }
        }
        cout << endl;
    }
}


/**
 * Starts solution 1
 */
void solution1(memory_t &data)
{
    Program program(&data);
    map<string, vector<cpp_int>> paintArea;
    vector<cpp_int> values;
    cpp_int actInput;
    while (program.state != P_HALT)
    {
        walk(&program, 0, 0, 0);
        break;
    }
}


/**
 * Starts solution 2
 *
 * Idea: Use a processing queue to process each inserted oxygen (and then its "childs"), until no more
 * oxygen needs to be produced.
 *
 * Start with the oxygen point. Create new oxygen in all 4 directions, until a wall is reached.
 * Register each new oxygen point in a "to process" list.
 * Now, process the "to process" list, do the same for each oxygen point, and register new oxygen.
 * Repeat until no more oxygen is in the process list.
 */
void solution2(Point *oStart, bool display)
{
    vector<Point *> processList;
    vector<Point *> nextToProcess;
    processList.push_back(oStart);
    int counter = 0;
    Point *act;
    while (processList.size() > 0)
    {
        nextToProcess.clear();
        for (auto oxy : processList)
        {
            // Check all 4 directions for an empty space:
            act = nullptr;
            // Up:
            if (oxy->y > 0)
            {
                act = floorMap[oxy->y - 1][oxy->x];
                if (act->object == ' ')
                {
                    act->object = 'O';
                    nextToProcess.push_back(act);
                }
            }
            // right:
            if (oxy->x < floorMap[0].size())
            {
                act = floorMap[oxy->y][oxy->x + 1];
                if (act->object == ' ')
                {
                    act->object = 'O';
                    nextToProcess.push_back(act);
                }
            }
            // down:
            if (oxy->y < floorMap.size())
            {
                act = floorMap[oxy->y + 1][oxy->x];
                if (act->object == ' ')
                {
                    act->object = 'O';
                    nextToProcess.push_back(act);
                }
            }
            // left
            if (oxy->x > 0)
            {
                act = floorMap[oxy->y][oxy->x - 1];
                if (act->object == ' ')
                {
                    act->object = 'O';
                    nextToProcess.push_back(act);
                }
            }
        }
        if (display)
        {
            printMap();
            std::chrono::milliseconds timespan(30);
            std::this_thread::sleep_for(timespan);
        }
        counter++;
        processList.clear();
        processList = nextToProcess;
    }
    counter--; // one too much, last does not count
    if (display)
    {
        printMap();
    }
    cout << "Solution 2: Room filled after " << counter << " Minutes" << endl;
}

int main(int argc, char *args[])
{
    bool display = argc >= 3 && string("--display").compare(args[2]) == 0 ? true : false;
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    memory_t data;
    readIntcodeProgramFromFile(args[1], data);

    solution1(data);

    Point *oxygenStart = buildFloorMap();
    if (display)
    {
        printMap();
    }
    solution2(oxygenStart, display);
}
