#include <iostream>
#include <vector>
#include <map>
#include <numeric>
#include <string>
#include <sstream>
#include <fstream>
#include "common.h"

using namespace std;

/**
 * Day 12
 */

unsigned long globalStepCounter = 0;

class Moon
{
public:
    long x = 0, y = 0, z = 0, vx = 0, vy = 0, vz = 0;
    map<string, string> seenCoordsX;
    map<string, string> seenCoordsY;
    map<string, string> seenCoordsZ;

    // Round-trip time for each coordinate, plus total:
    unsigned long rtX = 0;
    unsigned long rtY = 0;
    unsigned long rtZ = 0;
    unsigned long rtAll = 0;
};

void applyGravity(Moon *a, Moon *b)
{
    if (a->x < b->x)
    {
        a->vx++;
        b->vx--;
    }
    if (a->x > b->x)
    {
        a->vx--;
        b->vx++;
    }

    if (a->y < b->y)
    {
        a->vy++;
        b->vy--;
    }
    if (a->y > b->y)
    {
        a->vy--;
        b->vy++;
    }

    if (a->z < b->z)
    {
        a->vz++;
        b->vz--;
    }
    if (a->z > b->z)
    {
        a->vz--;
        b->vz++;
    }
}

void applyVelocity(Moon *a)
{
    a->x = a->x + a->vx;
    a->y = a->y + a->vy;
    a->z = a->z + a->vz;
}

/**/
void checkSamePos(Moon *m)
{
    if (m->rtAll > 0)
    {
        // done for this moon, we already know when it is back on field 1:
        return;
    }
    // X-coords-check:
    string key = "";
    key.append(to_string(m->x));key.append(":");key.append(to_string(m->vx));
    if (m->seenCoordsX.count(key) == 0)
    {
        // Not yet been here. Store position/velocity hash:
        m->seenCoordsX[key] = key;
    }
    else
    {
        // we were already here! Save the round-trip steps if not yet done:
        if (m->rtX == 0)
        {
            m->rtX = globalStepCounter;
        }
    }

    // Y-coords-check:
    key = "";
    key.append(to_string(m->y));key.append(":");key.append(to_string(m->vy));
    if (m->seenCoordsY.count(key) == 0)
    {
        // Not yet been here. Store position/velocity hash:
        m->seenCoordsY[key] = key;
    }
    else
    {
        // we were already here! Save the round-trip steps if not yet done:
        if (m->rtY == 0)
        {
            m->rtY = globalStepCounter;
        }
    }
    // Z-coords-check:
    key = "";
    key.append(to_string(m->z));key.append(":");key.append(to_string(m->vz));
    if (m->seenCoordsZ.count(key) == 0)
    {
        // Not yet been here. Store position/velocity hash:
        m->seenCoordsZ[key] = key;
    }
    else
    {
        // we were already here! Save the round-trip steps if not yet done:
        if (m->rtZ == 0)
        {
            m->rtZ = globalStepCounter;
        }
    }

    // Have we reached the same position for all coords?
    if (m->rtX > 0 && m->rtY > 0 && m->rtZ > 0)
    {
        cout << "Moon roundtrip reached: " << m->rtX << ":" << m->rtY << ":" << m->rtZ << endl;
        m->rtAll = std::lcm(m->rtX, m->rtY);
        m->rtAll = std::lcm(m->rtAll, m->rtZ);
    }
}
/**/

void calcStep(vector<Moon *> &moons)
{
    // Calc Velocity of all moons, by pairing each:
    for (unsigned int a = 0; a < moons.size(); ++a)
    {
        for (unsigned int b = a + 1; b < moons.size(); ++b)
        {
            applyGravity(moons[a], moons[b]);
        }
        applyVelocity(moons[a]);
        checkSamePos(moons[a]);
    }
}

long moonEnergy(Moon *a)
{
    return (abs(a->x) + abs(a->y) + abs(a->z)) *
           (abs(a->vx) + abs(a->vy) + abs(a->vz));
}

/**
 * Check if all moons have reachted its old positions at least once
 */
bool checkAllMoonsPositions(vector<Moon *> &moons)
{
    for (auto m : moons)
    {
        if (m->rtAll == 0)
        {
            return false;
        }
    }
    return true;
}


int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<string> fileData;
    readLines(args[1], fileData);

    // Create moons and set their data:
    vector<Moon *> moons;
    for (auto line : fileData)
    {
        line = line.substr(1, line.length() - 2);
        vector<string> parts;
        split(line, ',', parts);
        Moon *moon = new Moon();
        moon->vx = 0;
        moon->vy = 0;
        moon->vz = 0;
        for (unsigned int i = 0; i < parts.size(); ++i)
        {
            string part = trim(parts[i]);
            part = part.substr(2, part.length() - 2);
            if (i == 0)
            {
                moon->x = stol(part);
            }
            if (i == 1)
            {
                moon->y = stol(part);
            }
            if (i == 2)
            {
                moon->z = stol(part);
            }
        }
        // make sure the 1st coords are saved in the "already seen" coords maps:
        checkSamePos(moon);
        moons.push_back(moon);
    }
    // cout << "Moons created: " << moons.size() << endl;

    long energySum = 0;
    int steps = 1000;
    while (true)
    {
        globalStepCounter++;
        calcStep(moons);
        if (globalStepCounter == steps)
        {
            cout << "Solution 1: Total energy: " << energySum << endl;
        }
        // Solution 2:
        if (checkAllMoonsPositions(moons))
        {
            break;
        }
    }
    cout << "Calc done after " << globalStepCounter << " steps" << endl;
    unsigned long totalLCM = 1;
    for (auto m : moons)
    {
        cout << "LCM: " << m->rtAll << endl;
        totalLCM = std::lcm(totalLCM, m->rtAll);
    }
    cout << "Total needed steps for reaching start pos for all moons: "
         << totalLCM << endl;
}
