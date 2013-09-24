#ifndef POINT_H
#define POINT_H

#define EPS 0.0001

#include <iostream>
#include <cmath>

using namespace std;

struct Line {
    // The coeffience to form the line Ay + Bx = C
    double a;
    double b;
    double c;
};

class Point {
private:
    double xval, yval;
public:
    Point();
    Point(double x, double y);
    void setX(double x);
    void setY(double y);
    double x();
    double y();
    double dist(Point other);
    Point operator+(Point b);
    Point operator-(Point b);
    bool operator ==(Point p);
    bool operator <(Point p);
    void move(double a, double b);
    bool equal(Point p);

    void print(ostream &strm);
};

// Geometry functions
Point middle(Point a, Point b);
bool isLeft(Point a, Point b, Point c);
bool isOnLine(Point a, Line line);
bool collinear(Point a, Point b, Point c);
double grad(Point a, Point b);
double recip(double m);
Line getLine(Point a, Point b);
Line getPerpendic(Line line, Point at);
Point* intersect(Line line1, Line line2);
bool get_line_intersection(Point p0, Point p1, Point p2, Point p3, Point* inter);

#endif // POINT_H
