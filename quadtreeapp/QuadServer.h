//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef QUADSERVER_H_
#define QUADSERVER_H_

#include <set>          // std::set
#include <list>         // std::list
#include <vector>       // std::vector
#include <map>          // std::map
#include <algorithm>    // std::sort
#include "Point.h"
#include "Client.h"
#include "ConvexHullAlgs.h"

// Oversim
#include <OverlayKey.h>

// Defines
#define MAXCLIENTS 5
#define MINCLIENTS 1
#define WIDTH 800

class Vertex {
public:
    Vertex(Point a){
        loc = a;
        next = NULL;
    }
    ~Vertex() {
        delete &loc;
    }

    Point loc;
    Vertex* next;            // Pointer to next vertex in polygon
    Vertex* prev;
};

class Rectangle {
public:
    Rectangle(Point tL, Point bR) {
        topLeft = tL;
        botRight = bR;
    }

    Point topLeft;
    Point botRight;
};

struct Cell {
    int n;
    double rmax;
    Vertex* origin;             // pointer to origin of polygon Assume counter clockwise sequence
    std::list<Rectangle*> rect;  // A Set of point Rectangle objects each defining a rectangle
};

class QuadServer {
    public:
    QuadServer();
    ~QuadServer();
    QuadServer(OverlayKey k, double x, double y);
    QuadServer(OverlayKey k);

    void setMasterKey(OverlayKey k);
    void printNeighbourLocs();
    bool isLoaded();
    bool underLoaded();

/**********************
 *  QuadTree
 **********************/
    QuadServer(double x, double y, Point p1, Point p2);
    void addRect(Point p1, Point p2);
    void addRect(Rectangle *r);
    bool devide();              // Devide current rectangle into four and move location to top left rect
    bool merge();
    bool transfer(QuadServer* t);   // Transfer one of the current server's most loaded rectangles to t
    bool returnArea();          // Selects less loaded neighbour in same lvl, else on lvl up;
    bool insideArea(Point* tp);  // Determines if the test point tp is inside the area of current server
    bool adjacent(std::list<Rectangle*> *rect); // Determines if the Server t, and all its neighbours is adjacent to this, thus is a neighbour
    bool ownership(Client* c);
    std::vector<Client*> checkOwnership();

// Params
    OverlayKey key;
    OverlayKey masterKey;
    Point loc;
    int lvl;
    Cell cell;
    QuadServer* parent;
    int childCount;
    std::set<OverlayKey> neighbours;
    std::set<Client*> myClients;
};

bool ccw(Point p[], int n);
bool ccw(Point p1, Point p2, Point p3);
bool inRect(Point* tp, Rectangle* r);
void insertIntoVec(std::vector<Point> *v, Point p);

#endif /* QUADSERVER_H_ */
