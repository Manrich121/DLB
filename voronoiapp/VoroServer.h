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

#ifndef VOROSERVER_H_
#define VOROSERVER_H_

#include <set>          // std::set
#include <list>         // std::list
#include <vector>       // std::vector
#include <map>          // std::map
#include <algorithm>    // std::sort
#include "Point.h"
#include "Client.h"
#include "ConvexHullAlgs.h"
#include "VoronoiDiagramGenerator.h"

// Oversim
#include <OverlayKey.h>

// Defines
#define MAXCLIENTS 5
#define MINCLIENTS 1
//#define WIDTH 800

#define _DEBUG

class Vertex {
public:
    Vertex(Point a){
        loc = a;
        next = NULL;
    }
    ~Vertex() {
    }

    Point loc;
    Vertex* next;            // Pointer to next vertex in polygon
    Vertex* prev;
};

struct VCell {
    int n;
    double rmax;
    Vertex* origin;             // pointer to origin of polygon Assume counter clockwise sequence
};

class VoroServer {
public:
    VoroServer();
    VoroServer(OverlayKey k, int areaDim);
    ~VoroServer();
    VoroServer(OverlayKey k, double x, double y, int areaDim);

    void setMasterKey(OverlayKey k);
    void printNeighbourLocs();
    bool isLoaded();
    bool underLoaded();

    /**********************
     *  Distributed Voronoi
     **********************/
    void generateVoronoi();

    void GrahamSort(std::vector<Point> points);
    void GrahamScan(std::vector<Point> p);
    void vertsToVector(std::vector<Point> *v);
    void findIntersects(Line line, std::vector<Point> *ip);
    void checkNeighbours();
    bool isNeigh(Point tloc);

    void refine(VoroServer *t);     // calculate the new cell after intercection of the halfspace between
                                // the current server and new point
    Point getCenterofClients();
    void returnThisSite();
    void removeMe(OverlayKey t, map <OverlayKey, Point> excludeNeighs);
    bool ownership(Client* c);

    // Polygon functions
    void addVertex(Point a, bool ccw);
    void deleteCell();
    void deleteMyVertex(Vertex* v);
    bool pointInPolygon(Point p);
    std::vector<Point> *RemoveDup(std::vector<Point> v);

    // Params
    OverlayKey key;
    OverlayKey masterKey;
    Point loc;
    int lvl;
    VCell cell;
    VoroServer* parent;
    int childCount;
    std::map<OverlayKey, Point> neighbours;     // OverlayKey mapped to a server locations
    std::set<Client*> myClients;
    int areaDim;
};

bool ccw(Point p[], int n);
bool ccw(Point p1, Point p2, Point p3);
void insertIntoVec(std::vector<Point> *v, Point p);

#endif /* VOROSERVER_H_ */
