#include <iostream>
#include <string>
#include <vector>
#include "common.h"

using namespace std;

/**
 * Day 4 - Secure Container
 */

bool checkIncrease(const string &input)
{
    for (string::size_type i = 0; i < input.length() - 1; i++)
    {
        if (input[i] > input[i + 1])
        {
            return false;
        }
    }
    return true;
}

bool checkDigitPair(const string &input)
{
    for (string::size_type i = 0; i < input.length() - 1; i++)
    {
        if (input[i] == input[i + 1])
        {
            return true;
        }
    }
    return false;
}

bool checkDigitExactPair(const string &input)
{
    char nr = -1;
    char count = 0;

    for (string::size_type i = 0; i < input.length(); i++)
    {
        char actNr = input[i];
        if (actNr != nr)
        {
            if (count == 2)
            {
                return true;
            }
            count = 1;
            nr = actNr;
        }
        else
        {
            count++;
        }
    }
    return count == 2;
}

int main()
{
    int minInput = 206938;
    int maxInput = 679128;
    int solutions1 = 0;
    int solutions2 = 0;

    for (int act = minInput; act <= maxInput; act++)
    {
        string nrStr = to_string(act);
        if (checkIncrease(nrStr))
        {
            if (checkDigitPair(nrStr))
            {
                solutions1++;
            }
            if (checkDigitExactPair(nrStr))
            {
                solutions2++;
            }
        }
    }

    cout << "Solution 1: There are " << solutions1 << " numbers matching the criteria." << endl;
    cout << "Solution 2: There are " << solutions2 << " numbers matching the criteria." << endl;
}
