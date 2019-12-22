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
 * Day 13 - Care Package
 */

/**
 * Memory is a map of memory address(key) and value (value):
 */
typedef map<cpp_int, cpp_int> memory_t;

// Globals
int countBlockTiles = 0;
int width = 0;
int height = 0;
int ballX;
int paddleX;
// after some testing we can assume the field is not larger than that:
int field[50][50];

/**
 * Override program class to read inputs from Joystick values
 */
class JoystickInputProgram : public Program
{
    public:
        JoystickInputProgram(memory_t *mem) : Program(mem) {}

        cpp_int getInputValue() override {
            if (ballX < paddleX)
            {
                return cpp_int(-1);
            }
            if (ballX > paddleX)
            {
                return cpp_int(1);
            }
            return cpp_int(0);
        }
};



void printField()
{
    int tile;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            tile = field[x][y];
            if (tile == 0)
            {
                // empty
                cout << " ";
            }
            if (tile == 1)
            {
                // wall
                cout << "▦";
            }
            if (tile == 2)
            {
                // block
                cout << "▣";
            }
            if (tile == 3)
            {
                // Paddle
                cout << "▬";
            }
            if (tile == 4)
            {
                //Ball
                cout << "●";
            }
        }
        cout << endl;
    }
}

void solution1(memory_t &data)
{
    JoystickInputProgram program(&data);
    map<string, vector<cpp_int>> paintArea;
    vector<cpp_int> values;
    cpp_int actInput;
    while (program.state != P_HALT)
    {
        program.runProgram();
        if (program.state == P_OUTPUT)
        {
            // 1st output is the x-value:
            actInput = program.output;
            values.push_back(actInput);
            // start again, to wait for 2nd output (y value)
            program.runProgram();
            actInput = program.output;
            values.push_back(actInput);
            // start again, to wait for 3rd output (stone value)
            program.runProgram();
            actInput = program.output;
            values.push_back(actInput);
        }
        else
        {
            cout << "End of program" << endl;
            break;
        }
    }
    int tile = 0;
    int x = 0, y = 0;
    for (unsigned int i = 0; i < values.size(); i += 3)
    {
        x = static_cast<int>(values[i]);
        y = static_cast<int>(values[i + 1]);
        tile = static_cast<int>(values[i + 2]);
        if (x == 0)
        {
            cout << endl;
        }
        if (x > width)
            width = x;
        if (y > height)
            height = y;
        field[x][y] = tile;
        if (tile == 2)
        {
            // block
            countBlockTiles++;
        }
        if (tile == 3)
        {
            // Paddle
            paddleX = x;
        }
        if (tile == 4)
        {
            //Ball
            ballX = x;
        }
    }
    width++;
    height++;
    printField();
    cout << endl
         << "Field size: " << width << " x " << height << endl;
    cout << "Ball pos: " << ballX << endl;
    cout << "Paddle pos: " << paddleX << endl;
    cout << "Solution 1: " << countBlockTiles << endl;
}

void solution2(memory_t &data, bool withDisplay)
{
    // Insert free coin:
    data[cpp_int(0)] = 2;

    JoystickInputProgram program(&data);

    printField();

    // Play!
    int readX = 0, readY = 0, val = 0, points = 0;
    while (program.state != P_HALT)
    {
        program.runProgram();
        if (program.state == P_OUTPUT)
        {
            // 1st output is the x-value:
            readX = static_cast<int>(program.output);

            // 2nd value is the y-value
            program.runProgram();
            readY = static_cast<int>(program.output);

            // 3rd value is the tile value
            program.runProgram();
            val = static_cast<int>(program.output);

            // Point value is delivered: x=-1, y=0, value=points
            if (readX == -1 && readY == 0)
            {
                points = val;
            }
            else
            {
                if (val == 3)
                {
                    // paddle paint
                    paddleX = readX;
                }
                if (val == 4)
                {
                    // ball paint
                    ballX = readX;
                }
                // Update field, bookkeeping:
                if (field[readX][readY] == 2)
                {
                    // Block update
                    if (val != 2)
                    {
                        // Block destroyed;
                        countBlockTiles--;
                        // cout << "Block destroyed. Remaining: " << countBlockTiles << endl;
                    }
                }
                field[readX][readY] = val;
                if (withDisplay && val == 4) {
                    printField();
                    cout << "Points: " << points << endl;
                    std::chrono::milliseconds timespan(10);
                    std::this_thread::sleep_for(timespan);
                }
            }
        }
        else
        {
            cout << "End of program" << endl;
            break;
        }
    }
    cout << "Solution 2: Points after game has ended: " << points << endl;
}

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    memory_t data;
    readIntcodeProgramFromFile(args[1], data);

    solution1(data);
    solution2(data, argc >= 3 && string("--display").compare(args[2]) == 0 ? true : false);
}
