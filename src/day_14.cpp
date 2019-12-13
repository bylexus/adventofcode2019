#include <iostream>
#include <vector>
#include "common.h"

using namespace std;

/**
 * Day 14
 */

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<string> fileData;
    readData<string>(args[1], ',', fileData);
}
