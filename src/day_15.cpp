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
        runProgram(p);
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
                runProgram(p);
                continue;
            }

            // good, found the oxygen system somewhere ahead. Move on, to map the location
            if (result == 2)
            {
                p->inputValues.push(cpp_int(dirs[(i + 2) % 4]));
                runProgram(p);
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
    vector<long> fileData;
    memory_t data;
    readData<long>(args[1], ',', fileData);

    // convert input to big precision map:
    for (vector<long>::size_type i = 0; i < fileData.size(); ++i)
    {
        data[cpp_int(i)] = cpp_int(fileData[i]);
    }

    memory_t run1_data(data);
    // memory_t run2_data(data);
    solution1(run1_data);

    Point *oxygenStart = buildFloorMap();
    if (display)
    {
        printMap();
    }
    solution2(oxygenStart, display);
}
