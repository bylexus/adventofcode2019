#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include "common.h"
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
 * Memory is a map of memory address(key) and value (value):
 */
typedef map<cpp_int, cpp_int>
    memory_t;

/**
 * An AMP can be in one of the following states:
 * - P_RUN: still running
 * - P_OUTPUT: Program has generated an output, and is halted until continued
 * - P_HALT: The program is done, no furter run
 */
enum ProgramResult
{
    P_RUN,
    P_OUTPUT,
    P_HALT
};

class Program
{
public:
    int pNr = 0;
    memory_t *memory;
    cpp_int iPointer;
    queue<cpp_int> inputValues;
    cpp_int output;
    ProgramResult state;
    cpp_int relativeBase;

    Program(memory_t *mem)
    {
        memory = new memory_t(*mem);
        iPointer = cpp_int(0);
        relativeBase = cpp_int(0);
        state = P_RUN;
    }

    ~Program()
    {
        memory->clear();
        delete memory;
    }
};

/**
 * extracts the param modes from the whole opcode number.
 * Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
 * "10102": --> "101" are param modes, "02" is the opcode 2.
 * The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
 * the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
 *
 * The modes vector is filled with the param's opmode, in the CORRECT order (element 0: 1st param, element 1: 2nd param and so on)
 */
void readParamModesFromOpcode(cpp_int opcode, vector<int> &modes)
{
    string str = lexical_cast<string>(opcode);
    string modesStr{""};
    if (str.length() > 2)
    {
        modesStr = str.substr(0, str.length() - 2);
    }
    // cout << "  Input opcode:" << str << endl;
    for (vector<int>::size_type i = 0; i < modes.size(); i++)
    {
        if (i < modesStr.length())
        {
            char mode = modesStr[modesStr.length() - 1 - i];
            switch (mode)
            {
            case '0':
                modes[i] = 0;
                break;
            case '1':
                modes[i] = 1;
                break;
            case '2':
                modes[i] = 2;
                break;
            }
        }
    }
    // cout << "  Param modes" << modes[0] << ":"<<modes[1] <<":"<<modes[2] << endl;
}

/**
 * Reads data at the given pointer address, initializes it with zero if that
 * address is not yet initialized.
 */
cpp_int readValueFromMemory(memory_t &data, cpp_int iPointer)
{
    memory_t::iterator it = data.find(iPointer);
    if (it != data.end())
    {
        return it->second;
    }
    else
    {
        data[iPointer] = cpp_int(0);
        return data[iPointer];
    }
}

/**
 * Reads a value from memory:
 * - iPointer is the address of the parameter to read.
 * - The read parameter is either a direct value or a pointer, depending on the param's opmode.
 *
 * if paramMode is 0, this means position mode: The value at iPointer is a memory address to the actual value
 * if paramMode is 1, this means immediate mode: The value at iPointer is the actual value
 * if paramMode is 2, this means relative position mode: like position mode, but with a relative offset
 */
cpp_int getValue(Program *program, cpp_int iPointer, int paramMode)
{
    memory_t &data = *(program->memory);
    cpp_int paramValue = data[iPointer];
    if (paramMode == 0)
    {
        // position mode: data[iPointer] is a position pointer
        return readValueFromMemory(data, paramValue);
    }
    else if (paramMode == 1)
    {
        // immediate mode: data[iPointer] is the value
        return paramValue;
    }
    else if (paramMode == 2)
    {
        // relative mode: like position mode, but with a relative offset:
        return readValueFromMemory(data, paramValue + program->relativeBase);
    }
    else
    {
        cerr << "Unknown param mode: " << paramMode << endl;
        exit(1);
    }
}

/**
 * Calculates the store address for a write operation.
 *
 * iPointer is the address of a parameter to read. Depending on the paramMode, this is either
 * an absolute value or must be offset by the actual relativeBase.
 */
cpp_int getStorePos(Program *program, cpp_int iPointer, int paramMode)
{
    // No immediate mode for writing.
    memory_t &data = *(program->memory);
    cpp_int paramValue = data[iPointer];
    if (paramMode == 2)
    {
        // relative mode: memory address with offset
        return paramValue + program->relativeBase;
    }
    else
    {
        return paramValue;
    }
}

/**
 * Executes a single instruction (the next according to the Program's iPointer),
 * sets the output value (if applicable) and sets the iPointer to the next program
 * address.
 */
void executeNextInstruction(Program *program)
{
    // Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
    // "10102": --> "101" are param modes, "02" is the opcode 2.
    // The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
    // the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
    memory_t &data = *(program->memory);
    int opcode = static_cast<int>(data[program->iPointer] % 100); // last 2 digits are the opcode
    // cout << "I:" << program->iPointer << ": OP:" << opcode << "(" << data[program->iPointer] << ")" << endl;

    vector<int> paramModes{0, 0, 0};
    readParamModesFromOpcode(data[program->iPointer], paramModes);

    cpp_int val1;
    cpp_int val2;
    cpp_int storePos;
    cpp_int inputValue;
    switch (opcode)
    {
    // Add instr:
    case 1:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
        data[storePos] = val1 + val2;
        program->iPointer += 4;
        break;
    // Multiply instr:
    case 2:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
        data[storePos] = val1 * val2;
        program->iPointer += 4;
        break;
    // Input instr.
    case 3:
        inputValue = program->inputValues.front();
        program->inputValues.pop();
        storePos = getStorePos(program, program->iPointer + 1, paramModes[0]);
        data[storePos] = inputValue;
        // cout << "    Input value taken: " << inputValue << ", store at: " << storePos << endl;
        program->iPointer += 2;
        break;
    // Output instr.
    case 4:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        // cout << "    Output: " << val1 << endl;
        program->output = val1;
        program->iPointer += 2;
        program->state = P_OUTPUT;
        break;
    // Jump if true:
    case 5:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        if (val1 != 0)
        {
            program->iPointer = val2;
        }
        else
        {
            program->iPointer += 3;
        }
        break;
    // Jump if false:
    case 6:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        if (val1 == 0)
        {
            program->iPointer = val2;
        }
        else
        {
            program->iPointer += 3;
        }
        break;
    // less than
    case 7:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
        if (val1 < val2)
        {
            data[storePos] = 1;
        }
        else
        {
            data[storePos] = 0;
        }
        program->iPointer += 4;
        break;
    // equals:
    case 8:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
        if (val1 == val2)
        {
            data[storePos] = 1;
        }
        else
        {
            data[storePos] = 0;
        }
        program->iPointer += 4;
        break;
    // modify relative base:
    case 9:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        program->relativeBase += val1;
        program->iPointer += 2;
        break;
    case 99:
        // program->iPointer = data.size();
        program->state = P_HALT;
        break;
    default:
        cerr << "Error: unknown opcode occured: " << opcode << endl;
        exit(1);
    }
    // cout << endl;
}

/**
 * Run a program until the end, instruction-by-instruction, until it is set to
 * output or halt.
 */
void runProgram(Program *program)
{
    program->state = P_RUN;
    while (program->state == P_RUN)
    {
        // cout << "P:" << program->pNr << endl;
        executeNextInstruction(program);
        // cout << "E:" << program->pNr <<  ", state: " << program->state << endl;
    }
    // cout << "Exit: " << program->pNr << endl;
}

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
            runProgram(&program);
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
    runProgram(&program);
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
    vector<long> fileData;
    memory_t data;
    readData<long>(args[1], ',', fileData);

    // convert input to big precision map:
    for (vector<long>::size_type i = 0; i < fileData.size(); ++i)
    {
        data[cpp_int(i)] = cpp_int(fileData[i]);
    }

    memory_t run1_data(data);
    memory_t run2_data(data);
    int nrOfBeamPos = solution1(run1_data);
    printMap();
    cout << "Solution 1: Nr of Tractor pos: " << nrOfBeamPos << endl;
    int coordProd = solution2(run1_data);
    cout << "Solution 2: Ship Fit Coord Product: " << coordProd << endl;

}
