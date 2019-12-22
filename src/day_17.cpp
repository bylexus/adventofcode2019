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

class Point {
    public:
        Coord coord{0,0};
        char value = 0;
        bool visited = false;

        Point(int x, int y, char v): value{v} {
            coord = Coord(x, y);
        }
        Point() {

        }
};

typedef vector<Point> map_line_t;
typedef vector<map_line_t> map_t;
map_t scaffoldMap;
vector<Point> intersections;
Point robot;

/**
 * Day 17 - Set and Forget
 */


/**
 * Prints the map in its current state as ASCII Art
 */
void printMap()
{
    for (auto l : scaffoldMap) {
        for (auto c : l) {
            cout << c.value;
        }
        cout << endl;
    }
}

/**
 * Marks and collects intersections and the robot
 */
int analyzeMap(map_t &theMap)
{
    int sum = 0;
    // theMap[theMap.size()-1][theMap[theMap.size()-1].size()-1];
    cout << "max X:" << theMap[0].size()-1 << ", max Y: " << theMap.size()-1 << endl;
    for (map_t::size_type y = 0; y < theMap.size(); ++y) {
        for (map_t::size_type x = 0; x < theMap[y].size(); ++x) {
            // cout << "X:" << x << ", Y: " << y << endl;
            char value = theMap[y][x].value;
            if (value == 35) // '#
            {
                //look in all 4 dirs if there is also a '#': then it is an intersection
                if (y > 0 && x > 0 && y < theMap.size()-1 && x < theMap[y].size()-1)
                {
                    //    cout << "X:" << x << ", Y: " << y << endl;
                    if ( theMap[y-1][x].value == 35 &&
                            theMap[y+1][x].value == 35 &&
                            theMap[y][x-1].value == 35 &&
                            theMap[y][x+1].value == 35
                       ) {
                        //    theMap[y][x].value = 'O';
                        sum += y*x;
                        intersections.push_back(theMap[y][x]);
                    }
                }
            } else if (value == '^' || value == '<' || value == '>' || value =='v') {
                robot = theMap[y][x];
                robot.value = value == '^' ? 0 : (value == '>' ? 1 : (value == 'v' ? 2 : 3));
            }
        }
    }
    return sum;
}


/**
 * Starts solution 1
 */
void solution1(memory_t &data)
{
    Program program(&data);
    map_line_t line;
    int x = 0; int y = 0;
    while (program.state != P_HALT) {
        program.runProgram();
        char output = static_cast<char>(program.output);
        if (output != 10) {
            Point p(x, y, output);
            line.push_back(p);
            x++;
        } else {
            if (x > 0) {
                scaffoldMap.push_back(line);
                y++;
                x = 0;
                line = map_line_t();
            }
        }
    }
    int sumOfIntersections = analyzeMap(scaffoldMap);
    printMap();
    cout << "Nr of intersections: " << intersections.size() << endl;
    cout << "Robot position: " << robot.coord.x << ":" << robot.coord.y << endl;
    cout << "Solution 1: Intersections product sum: " << sumOfIntersections << endl;
}


/**
 * Starts solution 2
 */
void solution2(memory_t &data)
{
    // Start robot:
    data[0] = cpp_int(2);
    Program program(&data);
    string commands;
    // First, build a list of all movement functions, which will be split into smaller ones and
    // called by the main movement routine.

    int actWalkDir = robot.value; // 0 = up, 1 = right, 2 = down, 3 = left
    int moveCounter = 0;

    while (true) {
        int dir = robot.value;
        int x = robot.coord.x;
        int y = robot.coord.y;
        // Mark the position where the robot sits as visited:
        scaffoldMap[y][x].visited = true;

        // let the robot walk in the actual direction, if possible:
        // find the dir in which to let the robot run:
        if (dir == 0 && y > 0 && scaffoldMap[y-1][x].value == '#')
        {
            moveCounter++;
            robot.coord.y--;
        } else if (dir == 1 && x < scaffoldMap[y].size()-1 && scaffoldMap[y][x+1].value == '#')
        {
            moveCounter++;
            robot.coord.x++;
        } else if (dir == 2 && y < scaffoldMap.size()-1 && scaffoldMap[y+1][x].value == '#')
        {
            moveCounter++;
            robot.coord.y++;
        } else if (dir == 3 && x > 0 && scaffoldMap[y][x-1].value == '#')
        {
            moveCounter++;
            robot.coord.x--;
        } else {
            // Moving forward not possible, try to turn:
            // find the dir in which to let the robot run:
            actWalkDir = -1; // no direction
            if (y > 0 && scaffoldMap[y-1][x].value == '#' && scaffoldMap[y-1][x].visited == false)
            {
                actWalkDir = 0;
            } else if (x < scaffoldMap[y].size()-1 && scaffoldMap[y][x+1].value == '#' && scaffoldMap[y][x+1].visited == false)
            {
                actWalkDir = 1;
            } else if (y < scaffoldMap.size()-1 && scaffoldMap[y+1][x].value == '#' && scaffoldMap[y+1][x].visited == false)
            {
                actWalkDir = 2;
            } else if (x > 0 && scaffoldMap[y][x-1].value == '#' && scaffoldMap[y][x-1].visited == false)
            {
                actWalkDir = 3;
            }
            if (actWalkDir == -1) {
                // No more directions to walk. End.
                cout << "Reached final destination" << endl;
                break;
            }

            // Turn the robot, if necessary:
            if (actWalkDir != dir) {
                if (moveCounter > 0) {
                    commands += to_string(moveCounter) + ",";
                    moveCounter = 0;
                }
                int targetSourceDiff = actWalkDir - dir;
                if (targetSourceDiff == 1 || targetSourceDiff == -3) {
                    // Turn robot dir -> walkDir to right:
                    commands += "R,";
                } else if (targetSourceDiff == -1 || targetSourceDiff == 3)
                {
                    // Turn robot dir -> walkDir to left:
                    commands += "L,";
                }
            }
            robot.value = actWalkDir;
        }
    }
    if (moveCounter > 0) {
        commands += to_string(moveCounter);
    }
    cout << commands << endl;
    /**
     * Do splitting the output with pen-and-paper:
     * A: L,10,R,12,R,12,
     * B: R,6,R,10,L,10,
     * A: L,10,R,12,R,12,
     * C: R,10,L,10,L,12,R,6,
     * B: R,6,R,10,L,10,
     * C: R,10,L,10,L,12,R,6,
     * B: R,6,R,10,L,10,
     * C: R,10,L,10,L,12,R,6,
     * A: L,10,R,12,R,12,
     * C: R,10,L,10,L,12,R,6
     *
     * -->
     * Main movement:
     * A,B,A,C,B,C,B,C,A,C (19)
     * A: L,10,R,12,R,12
     * B: R,6,R,10,L,10
     * C: R,10,L,10,L,12,R,6
     */
    string main_command = "A,B,A,C,B,C,B,C,A,C";
    string function_a = "L,10,R,12,R,12";
    string function_b = "R,6,R,10,L,10";
    string function_c = "R,10,L,10,L,12,R,6";

    // Fill inputs:
    program.asciiInput(main_command);
    program.asciiInput(function_a);
    program.asciiInput(function_b);
    program.asciiInput(function_c);
    // No video feed:
    program.asciiInput("n");

    cpp_int last_output;
    while (program.state != P_HALT) {
        program.runProgram();
        last_output = program.output;
    }
    cout << "Solution 2: Output value: " << last_output << endl;
    cout << "**** NOTE ****: The 2nd solution is a Pen and Paper solution! It is NOT solved in a generic way." << endl;
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
    solution2(data);
}
