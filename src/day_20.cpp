#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "common.h"

using namespace std;

/**
 * Day 20 - Donut Maze
 *
 */

class Point
{
    public:
        Coord coord{0, 0};
        char value = 0;
        bool visited = false;
        bool isTeleport = false;
        string teleporter = "";
        int distance = INT_MAX;

        Point(int x, int y, char v) : value{v}
        {
            coord = Coord(x, y);
        }
        Point()
        {
        }
};

Point startPoint;
Point targetPoint;

/** Map of teleporters: name -> point1/2 */
map<string, vector<Point*>> teleporters;

typedef vector<Point*> map_line_t;
typedef vector<map_line_t> map_t;
map_t maze;

void printMap(map_t &maze)
{
    for (auto line : maze)
    {
        for (auto point : line) {
            cout << point->value;
        }
        cout << endl;
    }
}

void createTeleporter(Point* teleporterPoint, int x, int y, string name) {
    teleporterPoint->isTeleport = true;
    teleporterPoint->teleporter = name;
    teleporterPoint->coord.x = x;
    teleporterPoint->coord.y = y;

    if (teleporters.count(teleporterPoint->teleporter) == 0) {
        teleporters[teleporterPoint->teleporter] = vector<Point*>();
    }
    teleporters[teleporterPoint->teleporter].push_back(teleporterPoint);
}

void markTeleporter(map_t &maze, unsigned int x, unsigned int y)
{
    Point* teleport = nullptr;
    if (maze[y][x]->value == '.') {
        string teleportName = "  ";
        // Look for teleporters in all 4 directions:
        // Up:
        if (maze[y-1][x]->value >= 'A' &&
                maze[y-1][x]->value <= 'Z' &&
                maze[y-2][x]->value >= 'A' &&
                maze[y-2][x]->value <= 'Z') {
            teleportName[0] = maze[y-2][x]->value;
            teleportName[1] = maze[y-1][x]->value;
            createTeleporter(maze[y-1][x], x,y,teleportName);
            teleport = maze[y-1][x];
        }
        // down
        if (maze[y+1][x]->value >= 'A' &&
                maze[y+1][x]->value <= 'Z' &&
                maze[y+2][x]->value >= 'A' &&
                maze[y+2][x]->value <= 'Z') {
            teleportName[0] = maze[y+1][x]->value;
            teleportName[1] = maze[y+2][x]->value;
            createTeleporter(maze[y+1][x], x,y,teleportName);
            teleport = maze[y+1][x];
        }
        // left
        if (maze[y][x-1]->value >= 'A' &&
                maze[y][x-1]->value <= 'Z' &&
                maze[y][x-2]->value >= 'A' &&
                maze[y][x-2]->value <= 'Z') {
            teleportName[0] = maze[y][x-2]->value;
            teleportName[1] = maze[y][x-1]->value;
            createTeleporter(maze[y][x-1], x,y,teleportName);
            teleport = maze[y][x-1];
        }
        // right
        if (maze[y][x+1]->value >= 'A' &&
                maze[y][x+1]->value <= 'Z' &&
                maze[y][x+2]->value >= 'A' &&
                maze[y][x+2]->value <= 'Z') {
            teleportName[0] = maze[y][x+1]->value;
            teleportName[1] = maze[y][x+2]->value;
            createTeleporter(maze[y][x+1],x,y,teleportName);
            teleport = maze[y][x+1];
        }
    }
}

/**
 * Analyze the input:
 * - the maze begins at x=2, y=2, the outer border is reserved for teleport ident chars
 * - find all '.', and check if they are adjacent to a teleporter.
 * - register each teleport entry point in the map
 */
void analyzeMaze(map_t &maze) {
    for (map_t::size_type y = 2; y < maze.size(); ++y)
    {
        for (map_line_t::size_type x = 2; x < maze[y].size(); ++x)
        {
            if (maze[y][x]->value == '.') {
                markTeleporter(maze, x, y);
            }
            if (maze[y][x]->value == '#') {
                maze[y][x]->visited = true;
            }
            if (maze[y][x]->value == ' ') {
                maze[y][x]->visited = true;
            }
        }
    }
}

Point* teleport(Point *actPoint)
{
    vector<Point*> points = teleporters[actPoint->teleporter];
    Point* p = nullptr;
    if (points[0]->coord == actPoint->coord) {
        p = points[1];
    } else {
        p = points[0];
    }
    p->visited = true;
    return maze[p->coord.y][p->coord.x];
}

void walkMaze(Point *actPoint, int distance)
{
    if (actPoint->visited == true) {
        if (actPoint->distance <= distance) {
            return;
        }
    }
    if (actPoint->distance > distance) {
        actPoint->distance = distance;
    }

    actPoint->visited = true;

    // Have we reached the end?
    Point* target = teleporters["ZZ"][0];
    if (actPoint == target)
    {
        cout << "Reached ZZ with distance: " << distance << endl;
        return;
    }
    // are we on a teleporter? move along to the target point:
    if (actPoint->isTeleport == true) {
        Point *targetTeleportPoint = teleport(actPoint);
        actPoint = maze[targetTeleportPoint->coord.y][targetTeleportPoint->coord.x];
        return walkMaze(actPoint, distance);
    }

    // Try to walk in all 4 directions, check the next block recursively (depth-first):
    // up:
    Point* next = maze[actPoint->coord.y-1][actPoint->coord.x];
    if (next->value != '#' && next->teleporter != "AA") {
        walkMaze(next, distance+1);
    }
    // down:
    next = maze[actPoint->coord.y+1][actPoint->coord.x];
    if (next->value != '#' && next->teleporter != "AA") {
        walkMaze(next, distance+1);
    }
    // left:
    next = maze[actPoint->coord.y][actPoint->coord.x-1];
    if (next->value != '#' && next->teleporter != "AA") {
        walkMaze(next, distance+1);
    }
    // right:
    next = maze[actPoint->coord.y][actPoint->coord.x+1];
    if (next->value != '#' && next->teleporter != "AA") {
        walkMaze(next, distance+1);
    }
}


/**
 * Traverse the map with a backtrack depth-first algorithm.
 * Mark each coordinate with the (shortest) distance to the start point AA.
 * Map the whole maze, so that we have a full map of points to the end point ZZ, all with
 * (shortest) distances to the start point. We then already have the shortest distance.
 *
 * Teleporters are just traversed as null-distance points.
 *
 * That's Dyikstra!
 */
void solution1()
{
    Point* startPoint = teleporters["AA"][0];
    Point* targetPoint = teleporters["ZZ"][0];

    walkMaze(maze[startPoint->coord.y][startPoint->coord.x], 0);
    int dist = targetPoint->distance - 1;
    cout << "End walking maze" << endl;
    cout << "Solution 1: Shortest distance to target point is: " << dist << endl;
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

    // convert input to map
    for (vector<string>::size_type y = 0; y < inputLines.size(); ++y)
    {
        string line = inputLines[y];
        map_line_t mapLine;
        for (vector<string>::size_type x = 0; x < inputLines[y].size(); ++x) {
            Point* p = new Point(x,y,line[x]);
            mapLine.push_back(p);
        }
        maze.push_back(mapLine);
    }
    analyzeMaze(maze);
    for (auto it = teleporters.begin(); it != teleporters.end(); ++it)
    {
        string cname = it->first;
        cout << "Teleporters for " << cname << ": ";
        for (auto p = it->second.begin(); p != it->second.end(); ++p )
        {
            cout << "|" << (*p)->coord.x << ":" << (*p)->coord.y;
        }
        cout << endl;
    }
    printMap(maze);
    solution1();
}
