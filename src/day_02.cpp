#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include "common.h"

using namespace std;

/**
 * Day 2 - Program Alarm
 */

long runProgram(vector<long> &data)
{
    int programPointer = 0;
    int val1Pos;
    int val2Pos;
    long val1;
    long val2;
    int storePos;
    while (programPointer < data.size()) {
        int opcode = data[programPointer];
        switch (opcode) {
            case 1:
                val1Pos = data[programPointer+1];
                val2Pos = data[programPointer+2];
                val1 = data[val1Pos];
                val2 = data[val2Pos];
                storePos = data[programPointer+3];
                data[storePos] = val1 + val2;
            break;
            case 2:
                val1Pos = data[programPointer+1];
                val2Pos = data[programPointer+2];
                val1 = data[val1Pos];
                val2 = data[val2Pos];
                storePos = data[programPointer+3];
                data[storePos] = val1 * val2;
            break;
            case 99: programPointer = data.size(); break;
            default:
                cerr << "Error: unknown opcode occured: " << opcode << endl;
                exit(1);
        }
        programPointer += 4;
    }
    return data[0];
}

int main(int argc, char *args[])
{
    if (argc < 2) {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<long> data;
    readData<long>(args[1], ',',data);



    // Solution 1: Modify noun = 12, verb = 2:
    // Modify input to simulate error:
    vector<long> workingData(data);
    workingData[1] = 12;
    workingData[2] = 2;
    runProgram(workingData);
    cout << "Solution 1: Data at pos 0 after program end: " << workingData[0] << endl;


    // Solution 2: Modify both noun / verb from 0 - 99, to find solution 19690720
    long solution = 19690720;
    for(long noun = 0; noun <= 99; noun++) {
        for (long verb = 0; verb <= 99; verb++) {
            vector<long> workingData(data);
            workingData[1] = noun;
            workingData[2] = verb;
            if (runProgram(workingData) == solution) {
                cout << "Solution 2: Found solution " << solution << " with noun " << noun << ", verb " << verb << ", solution: 100*noun+verb = " << (100*noun+verb) << endl;
                exit(0);
            }
        }
    }
    cout << "ERROR: No solution found!" << endl;
}
