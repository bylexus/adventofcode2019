#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include "common.h"
#include "intcode.h"
#include <boost/multiprecision/cpp_int.hpp>
#include "CImg.h"

using namespace std;
using namespace boost::multiprecision;
using namespace boost;
using namespace cimg_library;

/**
 * Day 11 - Space Police
 */


void start(memory_t &data, cpp_int startInput, const string& imgName) {
    Program program(&data);
    map<string,cpp_int> paintArea;
    char dirs[] = {'U', 'R', 'D', 'L'};
    int dirIndex = 0;
    int coordX = 0, coordY = 0;
    int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
    cpp_int actInput;
    bool firstRun = true;
    while (program.state != P_HALT) {
        // read the input (color) on the actual coords.
        // 0 / not set means black, 1 means white
        string coordKey = to_string(coordX) + ":" + to_string(coordY);
        map<string,cpp_int>::iterator findIt = paintArea.find(coordKey);
        if (findIt == paintArea.end()) {
            if (firstRun) {
                actInput = startInput;
            } else {
                actInput = cpp_int(0);
            }
        } else {
            actInput = findIt->second;
        }
        firstRun = false;
        // fill the actual color as program input, execute:
        program.inputValues.push(actInput);
        program.runProgram();
        if (program.state == P_OUTPUT) {
            // 1st output is the color to be painted:
            actInput = program.output;
            paintArea[coordKey] = actInput;
            if (coordX < minX) minX = coordX;
            if (coordX > maxX) maxX = coordX;
            if (coordY < minY) minY = coordY;
            if (coordY > maxY) maxY = coordY;
        } else {
            cout << "End of program" << endl;
            break;
        }
        // start again, wait for 2nd output, the dir:
        program.runProgram();
        if (program.state == P_OUTPUT) {
            // 2nd output is the dir to turn:
            actInput = program.output;
            if (actInput == 0) {
                // turn left
                dirIndex = (dirIndex + 3) % 4;
            } else if (actInput == 1) {
                // turn right
                dirIndex = (dirIndex + 1) % 4;
            } else {
                cerr << "Oops, unknown direction output! die!" << endl; exit(1);
            }
            // calc new coords:
            if (dirs[dirIndex] == 'U') {
                coordY--;
            }
            if (dirs[dirIndex] == 'R') {
                coordX++;
            }
            if (dirs[dirIndex] == 'D') {
                coordY++;
            }
            if (dirs[dirIndex] == 'L') {
                coordX--;
            }
        } else {
            cerr << "Oops, premature end of program! Not good!" << endl; exit(1);
        }

    }
    int dX = maxX - minX;
    int dY = maxY - minY;

    // Draw an image / ASCII of the output
    CImg<unsigned char> pngImg(dX+4, dY+4, 1, 3, 0); // add a small border of 2 px per side
    for (int y = minY; y <= maxX; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            string coordKey = to_string(x) + ":" + to_string(y);
            map<string,cpp_int>::iterator findIt = paintArea.find(coordKey);
            if (findIt != paintArea.end() && findIt->second == 1) {
                cout << "▦";
                pngImg(x-minX+2, y-minY+2, 0, 0) = 255;
                pngImg(x-minX+2, y-minY+2, 0, 1) = 255;
                pngImg(x-minX+2, y-minY+2, 0, 2) = 255;
            } else {
                cout << " ";
            }
        }
        cout << endl;
    }
    cout << "Number of painted tiles: " << paintArea.size() << endl;
    pngImg.save_png(imgName.c_str());
    cout << "Output image saved to: " << imgName << endl;
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

    // Solution 1:
    cout << "Solution 1" << endl;
    start(data, cpp_int(0), "day_11_solution1.png");
    cout << endl <<endl << "Solution 2" << endl;

    // Solution 2:
    start(data, cpp_int(1), "day_11_solution2.png");
}
