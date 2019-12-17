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
        runProgram(&program);
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
    for (auto c : main_command) {
        program.inputValues.push(cpp_int(c));
    }
    program.inputValues.push(cpp_int('\n'));
    for (auto c : function_a) {
        program.inputValues.push(cpp_int(c));
    }
    program.inputValues.push(cpp_int('\n'));
    for (auto c : function_b) {
        program.inputValues.push(cpp_int(c));
    }
    program.inputValues.push(cpp_int('\n'));
    for (auto c : function_c) {
        program.inputValues.push(cpp_int(c));
    }
    program.inputValues.push(cpp_int('\n'));
    // No video feed:
    program.inputValues.push(cpp_int('n'));
    program.inputValues.push(cpp_int('\n'));

    cpp_int last_output;
    while (program.state != P_HALT) {
        runProgram(&program);
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
    solution1(run1_data);
    // printMap();
    solution2(run1_data);
}
