#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include <cstdlib>
#include "common.h"

using namespace std;

class Point;
class Segment;
typedef vector<Segment> wire_t;
typedef vector<wire_t> wire_container_t;
typedef wire_container_t::iterator wire_container_it_t;

    /**
 * Day 3 - Crossed Wires
 *
 * This solution is not optimized: the basic idea is to define horizontal and vertical
 * line segments which form the whole "wire". Then I have to check all segments of all wires
 * for intersections with all other segments of all other wires: This makes a O(n^4) loop neccessary.
 *
 * It works for this example, as there are only some few lines / segments (quite fast, < 10ms),
 * but it could fast be not feasible anymore.
 */

class Point
{
public:
    int x, y;

    Point()
    {
        x = 0;
        y = 0;
    }

    Point(int x, int y) : x(x), y(y) {}

    int manhattanDist(const Point &other)
    {
        return abs(this->x - other.x) + abs(this->y - other.y);
    }
};

/**
 * A wire segment: a vertical or horizontal segment of the whole wire
 */
class Segment {
    public:
        char dir; // The original direction, e.g. 'R'
        char align; // 'h' or 'v', indicates the line alignement
        Point p0; // The start point of the line, always nearer at Point 0/0
        Point p1; // The end point of the line
        int length;

        Segment(const string &def, int x, int y) {
            dir = def[0];
            p0.x = x;
            p0.y = y;
            p1.x = x;
            p1.y = y;

            istringstream(def.substr(1)) >> length;
            switch (dir) {
                case 'R':
                    p1.x = p0.x + length;
                    align = 'h';
                    break;
                case 'D':
                    p1.y = p0.y + length;
                    align = 'v';
                    break;
                case 'L':
                    p1.x = p0.x - length;
                    align = 'h';
                    break;
                case 'U':
                    p1.y = p0.y - length;
                    align = 'v';
                    break;
                default:
                    cerr << "Unknown dir. break.";
                    exit(1);
            }
        }

        Point* intersects(Segment &other) {
            // Lines that run in the same direction do not intersect:
            if (this->align == other.align) return nullptr;

            Segment *hSeg = nullptr;
            Segment *vSeg = nullptr;
            if (this->align == 'h') {
                hSeg = this;
                vSeg = &other;
            } else if (this->align == 'v') {
                hSeg = &other;
                vSeg = this;
            }
            int minX = hSeg->p0.x < hSeg->p1.x ? hSeg->p0.x : hSeg->p1.x;
            int maxX = hSeg->p0.x > hSeg->p1.x ? hSeg->p0.x : hSeg->p1.x;
            int minY = vSeg->p0.y < vSeg->p1.y ? vSeg->p0.y : vSeg->p1.y;
            int maxY = vSeg->p0.y > vSeg->p1.y ? vSeg->p0.y : vSeg->p1.y;

            if (minX <= vSeg->p0.x && maxX >= vSeg->p0.x && minY <= hSeg->p0.y && maxY >= hSeg->p0.y) {
                // yes, intersects. Return the intersection point, but only if this is not 0/0:
                if (vSeg->p0.x != 0 || hSeg->p0.y != 0) {
                    return new Point(vSeg->p0.x, hSeg->p0.y);
                }
            }
            return nullptr;
        }
};


int main(int argc, char *args[])
{
    if (argc < 2) {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<string> data;
    readData<string>(args[1], '\n',data);

    wire_container_t wires;


    // Split input:
    // Each line is a complete wire
    // each part (separated by comma) is a single segment
    int lastX, lastY;
    for(auto line : data) {
        lastX = 0; lastY = 0;
        vector<string> tokens;
        cout << "line: " << line << endl;
        wire_t wire;
        split(line, ',', tokens);
        for (auto token : tokens) {
            cout << "  " << token << endl;
            Segment segment(token, lastX, lastY);
            lastX = segment.p1.x;
            lastY = segment.p1.y;
            wire.push_back(segment);
        }
        wires.push_back(wire);
    }

    // Control output
    for (auto wire : wires) {
        cout << "New Wire:" << endl;
        for (auto segment : wire) {
            cout << "  " << segment.dir << ": S(" << segment.p0.x << "," << segment.p0.y << "), E(" << segment.p1.x << "," << segment.p1.y << "), L: " << segment.length << endl;
        }
    }

    // Run through all wires: check all segments of the actual wire for intersections with all other wire segments.
    Point *nearestIntersection = nullptr;
    int minWireDistToIntersection = 0;
    for (wire_container_it_t actWireIt = wires.begin(); actWireIt != wires.end(); ++actWireIt) {
        // counts the already iterated total length of the actual wire. Needed for Solution 2
        int actWireRunDist = 0;

        cout << "Working on wire: nr of segments: " << actWireIt->size() << endl;
        for (auto actSegment : *actWireIt) {
            // go through all following wires, and all their segments, and check for intersections:
            for (wire_container_it_t otherWireIt = actWireIt+1; otherWireIt != wires.end(); ++otherWireIt) {
                // counts the already iterated total length of the other wire. Needed for Solution 2
                int otherWireRunDist = 0;

                // Go through all segments of the other wire, and check for an intersection with
                // the actual wire's segment:
                for (auto otherSegment : *otherWireIt) {
                    Point *intersectionPoint = actSegment.intersects(otherSegment);
                    // Found an intersection:
                    if (intersectionPoint != nullptr) {
                        int dist = intersectionPoint->manhattanDist(Point(0,0));
                        if (nearestIntersection == nullptr) {
                            nearestIntersection = intersectionPoint;
                        } else if (nearestIntersection->manhattanDist(Point(0,0)) > dist) {
                            nearestIntersection = intersectionPoint;
                        }
                        cout << "    Found intersection at: " << intersectionPoint->x << ":" << intersectionPoint->y << ", D: " << dist << endl;

                        // for both segments that intersects, calc the distance from the beginning
                        // of the segment to the intersection point:
                        int actLen = actWireRunDist + intersectionPoint->manhattanDist(actSegment.p0);
                        int otherLen = otherWireRunDist + intersectionPoint->manhattanDist(otherSegment.p0);
                        int distSum = actLen + otherLen;
                        if (minWireDistToIntersection == 0 || distSum < minWireDistToIntersection) {
                            minWireDistToIntersection = distSum;
                        }
                        cout << "    Sum of distances of both wires to intersection: " << distSum << endl;
                    }
                    // Sum up the already processed total wire length of the other wire:
                    otherWireRunDist += otherSegment.length;
                }
            }
            // Sum up the already processed total wire length of the actual wire:
            actWireRunDist += actSegment.length;
        }
    }
    if (nearestIntersection != nullptr) {
        cout << "Solution 1: Manhattan dist to nearest Point (" << nearestIntersection->x << ":" << nearestIntersection->y << "): " << nearestIntersection->manhattanDist(Point(0,0)) << endl;
        cout << "Solution 2: Fewest combined steps to intersection point: " << minWireDistToIntersection << endl;
    }
}
