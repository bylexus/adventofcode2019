#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

class Point
{
public:
    int x = 0;
    int y = 0;

    Point() : Point(0, 0)
    {
    }
    Point(int x, int y) : x(x), y(y)
    {
        cout << "Point constructed: " << x << ":" << y << endl;
    }
    bool operator<(const Point &p2) const {
        return x < p2.x || y < p2.y;
    }

    ~Point()
    {
        cout << "Point deconstructed: " << x << ":" << y << endl;
    }
};

void play_001_f1(vector<shared_ptr<Point>> &v)
{
    for (int i = 0; i < 10; i++)
    {
        v.push_back(shared_ptr<Point>(new Point(i, i * 10)));
    }
}

void play_001_f2(vector<shared_ptr<Point>> &v)
{
    for (vector<shared_ptr<Point>>::iterator it = v.begin(); it != v.end(); ++it)
    {
        cout << "X: " << (*it)->x << ", Y: " << (*it)->y << endl;
    }
}

/**
 * Using shared_ptr
 */
void play_001()
{
    vector<shared_ptr<Point>> v;
    play_001_f1(v);
    play_001_f2(v);

    vector<shared_ptr<Point>> v2 = v;
    play_001_f2(v2);

    cout << "Use count of pointer: " << v2[0].use_count() << endl;
}

/**
 * Play with maps, and own classes
 */
void play_002() {
    Point p1(1,1);
    Point p2(2,2);
    Point p3(1,2);
    Point p4(2,2);

    map<Point, int> m;
    m[p1] = 1;
    m[p2] = 2;
    m[p3] = 3;

}

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give nr as parameter on command line" << endl;
        exit(1);
    }

    string nr = string(args[1]);

    if (nr == "1")
    {
        play_001();
    }
    if (nr == "2")
    {
        play_002();
    }
}
