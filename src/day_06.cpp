#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include <map>
#include "common.h"

using namespace std;

/**
 * Day 6 - Universal Orbit Map
 */

typedef map<string, string> orbit_map_t;
typedef vector<string> path_t;

/**
 * Counts the orbit path traversals, recursively, from a start planet to a target planet, and fills
 * the traversed planets into a path vector.
 */
int orbitPath(const string &planet, const string &target, orbit_map_t &orbitMap, path_t* path)
{
    if (path != nullptr) {
        path->push_back(planet);
    }

    if (planet.compare(target) == 0)
    {
        return 0;
    }
    else
    {
        orbit_map_t::iterator it = orbitMap.find(planet);
        if (it != orbitMap.end())
        {
            string orbitParent = it->second;
            return orbitPath(orbitParent, target, orbitMap, path) + 1;
        }
    }
    return 0;
}

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    // Build a map: Outer Planet --> InnerPlanet:
    vector<string> data;
    path_t youPath;
    path_t sanPath;
    orbit_map_t orbitMap;
    int orbitCount = 0;
    readData<string>(args[1], '\n', data);
    for (auto line : data)
    {
        vector<string> planets;
        split(line, ')', planets);
        orbitMap.insert(pair<string, string>(planets[1], planets[0]));
    }

    // Traverse the tree backwards (from leafes to trunk (COM)), counting traverses:
    for (orbit_map_t::iterator it = orbitMap.begin(); it != orbitMap.end(); ++it)
    {
        string outer = it->first;
        string inner = it->second;
        // Fill 2 special orbit paths for YOU ans SAN, for the 2nd part
        // For the 1st part, the orbit paths are not relevant
        if (outer.compare("YOU") == 0)
        {
            orbitCount += orbitPath(outer, "COM", orbitMap, &youPath);
        }
        else if (outer.compare("SAN") == 0)
        {
            orbitCount += orbitPath(outer, "COM", orbitMap, &sanPath);
        }
        else
        {
            orbitCount += orbitPath(outer, "COM", orbitMap, nullptr);
        }
    }

    cout << "Solution 1: Total Orbit count: " << orbitCount << endl;

    /**
     * Idea for solution 2:
     * create orbital paths for both YOU and SAN planet.
     * Then find the (1st) intersection (where the branches meet),
     * and calculate the distances to this itersection.
     * Sum them up, and done.
     */
    cout << "Solution 2: " << endl;

    int youCount = 0;
    int sanCount = 0;
    // find 1st intersection planet of both orbit paths,
    // while counting both path distances to the intersection:
    for (auto youPlanet : youPath)
    {
        auto it = find(sanPath.begin(), sanPath.end(), youPlanet);
        if (it != sanPath.end())
        {
            cout << "Found intersecting planet: " << *it << endl;
            sanCount = distance(sanPath.begin(), it);
            const string intersection(*it);
            cout << "It takes " << youCount << " from you to intersection" << endl;
            cout << "It takes " << sanCount << " from san to intersection" << endl;
            cout << "Total it takes " << (youCount + sanCount - 2) << endl;
            break;
        }
        youCount++;
    }
}
