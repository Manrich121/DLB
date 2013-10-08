#include "Point.h"

Point::Point(){
}

Point::Point(double x = 0.0, double y = 0.0) {
    xval = x;
    yval = y;
}

// Setters

void Point::setX(double x) {
    xval = x;
}
void Point::setY(double y) {
    yval = y;
}

// Extractors
double Point::x() { return xval; }
double Point::y() { return yval; }

// Distance to another point.  Pythagorean thm.
double Point::dist(Point other) {
    double xd = xval - other.xval;
    double yd = yval - other.yval;
    return sqrt(xd*xd + yd*yd);
}

// Add or subtract two points.
Point Point::operator +(Point b)
{
    return Point(xval + b.xval, yval + b.yval);
}
Point Point::operator -(Point b)
{
    return Point(xval - b.xval, yval - b.yval);
}

bool Point::operator ==(Point p)
{
    return (this->x() == p.x()) && (this->y() == p.y());
}

bool Point::operator <(Point p) {
    return (xval < p.x() && yval < p.y());
}

// Move the existing point.
void Point::move(double a, double b)
{
    xval += a;
    yval += b;
}

// Print the point on the stream.  The class ostream is a base class
// for output streams of various types.
void Point::print(ostream &strm)
{
    strm << "(" << xval << "," << yval << ")";
}

// Retruns true if the two points are equal
bool Point::equal(Point p) {
    return (abs(this->x() - p.x()) <= 0.001 && abs(this->y() - p.y()) <= 0.001);
}


/****************************************
 *  Geometry functions
 ****************************************/

// Returns the midpoint between two points
Point middle(Point a, Point b) {
    return Point((a.x() + b.x()) / 2.0,
                 (a.y() + b.y()) / 2.0);
}

// Returns true if the point is left or on a ccw line
bool isLeft(Point a, Point b, Point c) {
    return ((b.x() - a.x())*(c.y() - a.y()) - (b.y() - a.y())*(c.x() - a.x())) > 0;
}

bool isOnLine(Point p, Line line) {
    return (p.y() == (line.c-line.b*p.x()));
}

// Returns whether 3 points are collinear and in order a->b->c
bool collinear(Point a, Point b, Point c) {
    if ((a.y() - b.y()) * (a.x() - c.x()) == (a.y() - c.y()) * (a.x() - b.x())) {
        return (a.dist(b) <= a.dist(c) && c.dist(b) <= c.dist(a));
    }
    return false;
}

// Calculates and return the gradient between two points
double grad(Point a, Point b) {
    return 1.0 * (a.y() - b.y()) / (a.x() - b.x());
}

// Return the reciprocal of the grad so that m1*m2=-1
double recip(double m) {
    return -1/m;
}

/*
 *  Calculates the equation of the line between two lines
 *  Ay + Bx = C
 */
Line getLine(Point v, Point w) {
    Line line;
    line.a = 1.0;                       // A is always chose as one
    line.b = -1*grad(v,w);              // B is equal to m, the gradiant of the two lines
    if (line.b == INFINITY || line.b == -INFINITY) { // x1 == x2
        line.a = 0;
        line.b = 1;
        line.c = v.x();                 // c = x1
    }else{
        if (line.b == 0) {              // horizontal line y=c
            line.c = v.y();
        }else{
            line.c =  line.a*v.y() + line.b*v.x();     // C is eqaul to m(-x1) + y1
        }
    }


    return line;
}

Line getPerpendic(Line line, Point at) {
    Line newLine;
    // if vertical newLine => horizontal
    if (line.a == 0){       // y=0; x=c
        newLine.a = 1;
        newLine.b = 0;
        newLine.c = at.y();
    }else{
        if (line.b == 0) {  // x=0; y=c
            newLine.a = 0;
            newLine.b = 1;
            newLine.c = at.x();
        }else{
            newLine.a = 1.0;
            newLine.b = -1*recip(-1*line.b);    // Find the reciprocal of line
            newLine.c = at.y() + newLine.b*at.x();       // Calculate the equation through the point 'at'
        }
    }
    return newLine;
}

/*  Calculate the intersection of two lines
*   A1y + B1x = C1
*   A2y + B2x = C2
*
*   Returns a NULL pointer if the two lines are parallel
*   **Ref: backstrom,“AlgorithmTutorials.”
*   [Online]: http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=geometry2#line_line_intersection
*/

Point* intersect(Line line1, Line line2) {
    double det = line1.b*line2.a - line2.b*line1.a;     // Calculate the determinant
    double x;
    double y;
    if (det == 0){                      // Lines are parallel
        return NULL;
    }else{                              // Calculate intersection point

        x = (line2.a*line1.c - line1.a*line2.c)/det;
        y = (line1.b*line2.c - line2.b*line1.c)/det;

        if(x==0.0) {
            x = 0.0;
        }
        if (y == 0.0) {
            y = 0.0;
        }
    }

    return new Point(x,y);
}

//Point* intersect(Line line1, Line line2) {
//    double x;
//    double y;
//    if (line1.a == 0){   // x1 = c1
//        x = line1.c;
//    }else{
//        if (line2.a == 0){   // x1 = c1
//            x = line2.c;
//        }else{
//            x = (line1.c - line2.c)/(line1.b - line2.b);
//        }
//    }

//    if (line2.b == 0) { // y2 = c2
//        y = line2.c;
//    }else{              // y2 = -b2x +c2
//        y = line2.c-x*line2.b;
//    }

//    return new Point(x,y);
//}


bool get_line_intersection(Point p0, Point p1, Point p2, Point p3, Point* inter) {
        //float p0_x, float p0_y, float p1_x, float p1_y,
    //float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)

    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1.x() - p0.x();
    s1_y = p1.y() - p0.y();
    s2_x = p3.x() - p2.x();
    s2_y = p3.y() - p2.y();

    float s, t;
    s = (-s1_y * (p0.x() - p2.x()) + s1_x * (p0.y() - p2.y())) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0.y() - p2.y()) - s2_y * (p0.x() - p2.x())) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
        if (inter != NULL)
            inter->setX(p0.x() + (t * s1_x));
            inter->setY(p0.y() + (t * s1_y));
        return true;
    }

    return false; // No collision
}

