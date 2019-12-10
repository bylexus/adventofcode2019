#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include "common.h"

using namespace std;

/**
 * Day 10 - Monitoring Station
 *
 */

class Asteroid
{
public:
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int nrOfViews = 0;
    double angle;
    unsigned int distance;
    bool alive = true;

    Asteroid(unsigned int x, unsigned int y) : x(x), y(y)
    {
    }
};

/**
 * I'm sure there is a more elegant and efficient way to calculate a 2*PI / 360°
 * angle of a directed line.... but for now that is the safe way.
 *
 * Also, I could use Radiants, but I can better imagine Degree angles in my head,
 * therefore the calcs:
 */
double calcAngle(Asteroid *from, Asteroid *to)
{
    int dx = abs((int)from->x - (int)to->x);
    int dy = abs((int)from->y - (int)to->y);

    // 0°: to is directly north from:
    if (dx == 0 && to->y < from->y)
        return 0.0;
    // 90°: to is horizontal right of from:
    if (dy == 0 && to->x > from->x)
        return 90.0;
    // 180°: to is directly south from from:
    if (dx == 0 && to->y > from->y)
        return 180.0;
    // 270°: to is directly west from from:
    if (dy == 0 && to->x < from->x)
        return 270.0;

    double slope = (double)dy / double(dx);

    // top right quadrant:
    if (to->x > from->x && to->y < from->y)
    {
        return 90 - atan(slope) * 180 / M_PI;
    }

    // bottom right quadrant:
    if (to->x > from->x && to->y > from->y)
    {
        return 90 + atan(slope) * 180 / M_PI;
    }

    // bottom left quadrant:
    if (to->x < from->x && to->y > from->y)
    {
        return 270 - atan(slope) * 180 / M_PI;
    }

    // top left quadrant:
    if (to->x < from->x && to->y < from->y)
    {
        return 270 + atan(slope) * 180 / M_PI;
    }

    cerr << "You should never reach this far!" << endl;
    exit(1);
    return 0.0;
}

int calcDistance(Asteroid *from, Asteroid *to)
{
    return abs((int)from->x - (int)to->x) + abs((int)from->y - (int)to->y);
}

/**
 * Idea: A line is a triangle of an x and y axis. y is a fn of x:
 * yd and yd is the distance in x/y values from one asteroid to the other,
 * then is an y for each x on that line:
 *
 * y = f(x) -> x * (yd / xd)
 */
bool hasLineOfSight(Asteroid *a, Asteroid *b, vector<vector<char>> &field)
{
    Asteroid *left = a->x <= b->x ? a : b;
    Asteroid *right = a->x <= b->x ? b : a;
    int dx = right->x - left->x;
    int dy = right->y - left->y;
    double slope = dx > 0 ? (double)dy / (double)dx : 0;
    double y = 0;
    double tmp;
    unsigned int testX, testY;
    // cout << "Test distances: x:" << dx << ", y: " << dy << endl;
    if (dx > 0)
    {
        // non-vertical line
        for (unsigned int x = 1; x < dx; ++x)
        {
            y = (double)x * slope;
            // cout << "Test x:y:slope " << x << ":" << y << ":" << slope << endl;
            // check for an exact (whole number) y:
            if (modf(y, &tmp) == 0)
            {
                testX = x + left->x;
                testY = (int)y + left->y;
                // cout << "Test x:y:slope " << testX << ":" << testY << ":" << slope << ":" << field[testY][testX]<< endl;
                // cout << "1: x: " << testX << "y: " << testX << endl;
                if (field[testY][testX] == '#')
                {
                    return false;
                }
            }
        }
    }
    else
    {
        // vertical line, just check y line
        for (unsigned int y = min(left->y, right->y) + 1; y < max(left->y, right->y); ++y)
        {
            if (field[y][left->x] == '#')
            {
                return false;
            }
        }
    }

    return true;
}

Asteroid *findNearest(vector<Asteroid *> &list)
{
    Asteroid *nearest = nullptr;
    for (auto a : list)
    {
        if (a->alive)
        {
            if (nearest == nullptr || nearest->distance > a->distance)
            {
                nearest = a;
            }
        }
    }
    return nearest;
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
    readData<string>(args[1], '\n', fileData);

    // field is the asteroid field, field[y][x]
    unsigned int width = fileData[0].length();
    unsigned int height = fileData.size();
    vector<vector<char>> field;
    vector<Asteroid *> asteroids;
    // read in field:
    for (vector<string>::size_type y = 0; y < height; ++y)
    {
        vector<char> line;
        for (string::size_type x = 0; x < width; ++x)
        {
            line.push_back(fileData[y][x]);
            if (fileData[y][x] == '#')
            {
                asteroids.push_back(new Asteroid(x, y));
            }
        }
        field.push_back(line);
    }

    // cout << "List of asteroids:" << endl;
    // int counter = 0;
    // for (auto a : asteroids)
    // {
    //     cout << counter++ << ": " << a->x << ":" << a->y << endl;
    // }

    // cout << "Asteroid field:" << endl;
    // for (vector<string>::size_type y = 0; y < height; ++y)
    // {
    //     for (string::size_type x = 0; x < width; ++x)
    //     {
    //         cout << field[y][x];
    //     }
    //     cout << endl;
    // }

    /**
     * Calculate the number of sights for each asteroid:
     * check every other asteroid if it is in the actual's line of sight.
     * If so, increates sight counter on both asteroids.
     * Repeat for every asteroid for all of its companions
     */
    for (unsigned int aI = 0; aI < asteroids.size(); ++aI)
    {
        for (unsigned int bI = aI + 1; bI < asteroids.size(); ++bI)
        {
            if (hasLineOfSight(asteroids[aI], asteroids[bI], field))
            {
                (asteroids[aI])->nrOfViews++;
                (asteroids[bI])->nrOfViews++;
            }
        }
    }

    // Find the one with the most counts:
    unsigned int maxSights = 0;
    Asteroid *bestAsteroid = nullptr;
    for (auto a : asteroids)
    {
        if (a->nrOfViews > maxSights)
        {
            maxSights = a->nrOfViews;
            bestAsteroid = a;
        }
    }
    cout << "Solution 1: Best asteroid is " << bestAsteroid->x << ":" << bestAsteroid->y << " with " << bestAsteroid->nrOfViews << " sights." << endl;

    // Solution 2
    /**
     * Brain dump:
     * 1. Calculate the angle and distance of the line from the best asteroid to all other asteroids, save it on the asteroid
     * 2. create a sorted list of all (unique) angles
     * 3. create a map of angles -> list of asteroids
     * 4. loop through the list of angles, take the nearest asteroid per angle, destroy it, count it
     * 5. loop until 200 asteroids are destroyed.
     */
    map<double, vector<Asteroid *>> angleMap;
    vector<double> angleList;

    // Calculate angle and distance to the best-placed asteroid from Solution 1:
    // Store the angle and distance values on each asteroid:
    // At the same time, create a list with all angles found,
    // and store each asteroid in an angle -> [asteroid] map,
    // to process them later:
    for (auto a : asteroids)
    {
        if (a != bestAsteroid)
        {
            a->distance = calcDistance(bestAsteroid, a);
            a->angle = calcAngle(bestAsteroid, a);
            if (angleMap.find(a->angle) == angleMap.end())
            {
                angleMap[a->angle] = vector<Asteroid *>();
                angleList.push_back(a->angle);
            }
            angleMap[a->angle].push_back(a);
        }
    }

    // Sort angle list, so that they can be processed from smallest to largest:
    sort(angleList.begin(), angleList.end());

    int asteroidCounter = 0;
    unsigned int angleIndex = 0;
    double actAngle = 0;
    Asteroid *act = nullptr;

    // Destroy asteroids, one by one (or, angle by angle, while destroying one per angle/round)
    while (asteroidCounter < 200)
    {
        actAngle = angleList[angleIndex];
        act = findNearest(angleMap[actAngle]);
        if (act != nullptr)
        {
            act->alive = false;
            asteroidCounter++;
        }
        angleIndex = (angleIndex + 1) % angleList.size();
    }

    cout << "Asteroid " << asteroidCounter << "found: " << act->x << ":" << act->y << endl;
    cout << "Solution 2: " << (act->x * 100 + act->y) << endl;

}
