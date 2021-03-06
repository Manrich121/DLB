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

#include "VoroServer.h"

#define EPS 0.001

VoroServer::VoroServer(){
    key = OverlayKey::ZERO;
    loc = Point(0,0);
    cell.n = 0;
    cell.origin = NULL;
}

VoroServer::VoroServer(OverlayKey k, int areaDim){
    key = k;
    loc = Point(0,0);
    cell.n = 0;
    cell.origin = NULL;
    this->areaDim = areaDim;
}

VoroServer::~VoroServer() {
}

VoroServer::VoroServer(OverlayKey k, double x, double y, int areaDim)
{
    key = k;
    loc = Point(x,y);
    cell.n = 0;
    cell.origin = NULL;
    this->areaDim = areaDim;
}

void VoroServer::setMasterKey(OverlayKey k) {
    this->masterKey = k;
}

bool VoroServer::isLoaded() {
    return this->myClients.size()>MAXCLIENTS;
}

bool VoroServer::underLoaded() {
        return this->myClients.size()<MINCLIENTS;
}

/************************************
 *  Distibuted Voronoi
 ***********************************/

std::vector<Point> myUnique(std::vector<Point> points) {
    Point curPoint;
    Point compPoint;
    std::vector<Point> uniques;
    bool dup;
    for (unsigned int i=0;i<points.size()-1; i++){
        dup = false;
        curPoint = points.at(i);
        for(unsigned int j=i+1;j<points.size();j++){
            compPoint = points.at(j);
            if(curPoint.equal(compPoint)) {
                dup = true;
                break;
            }
        }
        if(!dup)
            uniques.push_back(curPoint);
    }
    uniques.push_back(points.back());
    return uniques;
}

void VoroServer::refine(VoroServer* t) {
    t->loc = this->getCenterofClients();

    this->neighbours[t->key] = t->loc;
    t->neighbours[this->key] = this->loc;

    //transfer all my neighbours
    map<OverlayKey,Point>::iterator nit;
    for(nit=this->neighbours.begin();nit!=neighbours.end();nit++){
        if((*nit).first != t->key){
            t->neighbours[(*nit).first] = (*nit).second;
        }
    }
    t->generateVoronoi();
    this->generateVoronoi();

    set <Client*>::iterator cit;
    set<Client*> tmpSet = myClients;
    for (cit = this->myClients.begin(); cit != this->myClients.end();cit++) {
        if (t->ownership(*cit)){
            t->myClients.insert(*cit);
            tmpSet.erase(*cit);
        }
    }

    this->myClients = tmpSet;
}

Point VoroServer::getCenterofClients(){
    set <Client*>::iterator cit;
    double x=0;
    double y=0;

    for(cit = this->myClients.begin(); cit != this->myClients.end();cit++){
        x += (*cit)->loc.x();
        y += (*cit)->loc.y();
    }

    x /= this->myClients.size();
    y /= this->myClients.size();
    return Point(x,y);
}

int VoroServer::numCalc =0;

void VoroServer::generateVoronoi() {
    VoroServer::numCalc++;

    std::vector<Point> sPoints;
    std::vector<Point> vPoints;
    std::vector<Point> points;
    sPoints.clear();
    vPoints.clear();
    points.clear();
    VoronoiDiagramGenerator vdg;
    map<OverlayKey,Point>::iterator it;
    float x1,y1,x2,y2;
    Point curPoint;
    double distTp, newDist;

    // Get all server locations
    points.push_back(this->loc);
    for(it = this->neighbours.begin(); it != this->neighbours.end(); it++) {
        points.push_back((*it).second);
    }

    int count = points.size();
    float xValues[count];
    float yValues[count];

    vPoints.push_back(Point(0,0));
    vPoints.push_back(Point(areaDim,0));
    vPoints.push_back(Point(areaDim,areaDim));
    vPoints.push_back(Point(0,areaDim));

    for (int i=0;i<count;i++) {
        xValues[i] = points.at(i).x();
        yValues[i] = points.at(i).y();
    }

    vdg.generateVoronoi(xValues,yValues,count, 0,areaDim,0,areaDim);

    vdg.resetIterator();
//    printf("\n-------------------------------\n");
    while(vdg.getNext(x1,y1,x2,y2))
    {
//        printf("GOT Line (%g,%g)->(%g,%g)\n",x1,y1,x2, y2);
        vPoints.push_back(Point(x1,y1));
        vPoints.push_back(Point(x2,y2));
    }

    vPoints = myUnique(vPoints);
    this->deleteCell();
    bool mine;
    for (unsigned int i=0;i<vPoints.size();i++) {
        mine = true;
        curPoint = vPoints[i];
        distTp = this->loc.dist(curPoint);
        for(it = this->neighbours.begin(); it != this->neighbours.end(); it++) {
            newDist = (*it).second.dist(curPoint);
            if (abs(newDist - distTp) > EPS) {
                if (newDist < distTp) {
                    mine = false;
                }
            }

//            if(distTp < newDist){
//                mine = true;
//            }else if(abs(newDist - distTp) < EPS) {
//                mine = true;
//            }else if(newDist < distTp){
//                mine = false;
//            }
        }
        if (mine) {
            sPoints.push_back(curPoint);
        }
    }

    this->GrahamScan(sPoints);
}

/*
 *  Adds a Vertex to this.cell incrementing cell.n and calculate cell.rmax as the maximum distance
 *  from this.loc and the new Vertex
 */
void VoroServer::addVertex(Point a, bool ccw) {
    Vertex* pointer = this->cell.origin;
    Vertex* vNode = new Vertex(a);
    vNode->next = NULL;

    if (pointer == NULL) {          // Cell not yet init
        this->cell.origin = vNode;
        this->cell.origin->prev = NULL;
        this->cell.n = 1;
        this->cell.rmax = 0;
    }else{
        if (pointer->prev == NULL) {    // Cell only contains one Vertex=origin
            pointer->next = vNode;      // Add new Vertex
            this->cell.n++;
            vNode->prev = pointer;
        }else{
            pointer = this->cell.origin->prev;    // Jump to end of polygon
            if (ccw== true && collinear(pointer->prev->loc, pointer->loc, vNode->loc)){
               pointer->prev->next = vNode;
               vNode->prev = pointer->prev;
               this->cell.origin->prev = vNode;
               pointer->~Vertex();
               return;
           }
            pointer->next = vNode;      // Add new Vertex
            this->cell.n++;
            vNode->prev = pointer;
        }
        this->cell.origin->prev = vNode;
    }


    // Recalc rmax
    double newR = 2*this->loc.dist(a);    // calculate radius to new point
    if (newR > this->cell.rmax) {
        this->cell.rmax = newR;
    }
}

void VoroServer::deleteCell(){
    if (this->cell.origin == NULL) {
        return;
    }
    deleteMyVertex(this->cell.origin);
    this->cell.origin = NULL;
    this->cell.n = 0;
    this->cell.rmax =0;
}

void VoroServer::deleteMyVertex(Vertex* v) {
    if (v->next == NULL) {
        delete v;
    }else{
        deleteMyVertex(v->next);
    }
}

bool VoroServer::ownership(Client* c) {
    return this->pointInPolygon(c->loc);
}

bool VoroServer::isNeigh(Point tloc) {
    return this->loc.dist(loc) <= this->cell.rmax;
}

void VoroServer::findIntersects(Line line, std::vector<Point> *ip) {
    Line cellLine;
    Vertex* pointer = this->cell.origin;
    Point* tp;

    while(pointer->next != NULL){                   // While not at end of polygon
        cellLine = getLine(pointer->loc, pointer->next->loc);
        tp = intersect(line, cellLine);
        if (tp == NULL ) {
            pointer = pointer->next;
            continue;
        }
        // (a.dist(b) <= a.dist(c) && c.dist(b) <= c.dist(a))
        if (pointer->loc.dist(*tp) <= pointer->loc.dist(pointer->next->loc) &&
                pointer->next->loc.dist(*tp) <= pointer->next->loc.dist(pointer->loc)) {
            if (ip->size() <2) {
                ip->push_back(*tp);
            }else{
                return;
            }
        }
        pointer = pointer->next;
    }
    if (ip->size() <2) {
        // Last line segment
        cellLine = getLine(pointer->loc,this->cell.origin->loc);
        tp = intersect(line, cellLine);
        if (tp != NULL) {
            if (pointer->loc.dist(*tp) <= pointer->loc.dist(this->cell.origin->loc) &&
                    this->cell.origin->loc.dist(*tp) <= this->cell.origin->loc.dist(pointer->loc)) {
                ip->push_back(*tp);
            }
        }
    }
}

/*
 *  Construct a simple polygon by ustilising the sorting step of the Graham scan algorithm.
 *  Given a array of points, and sets this.cell equal to a simple ccw polygon
 *  First select the bottom-leftmost point l then sorts all following points ccw around L
 *  **Ref:  J. Erickson, “Lecture: Convex Hulls,” 2008.
 *          Available: www.cs.uiuc.edu/jeffe/teaching/compgeom/notes/01-convexhull.pdf
 */
void VoroServer::GrahamSort(std::vector<Point> points) {
    int lpos = 0;
    Point l = points[lpos];
    Point tmp, p1, p2;

    // Find top-leftmost point
    for (unsigned int i=1;i<points.size();i++) {
        if (points[i].x() <= l.x()) {     // Second mininum x
            l = points[i];
            lpos = i;
        }
    }
    // Remove l from vector points
    points.erase(points.begin()+lpos);

    this->addVertex(l,true);

    unsigned i;
    // Sort
    for (i = 0;i <points.size()-1;i++) {
        for(unsigned j = i+1; j<points.size(); j++) {
            p1 = points[i];
            p2 = points[j];
            if (!ccw(l, points[i], points[j])){     // If not ccw, swap so that it is counter clock wise;
                tmp = points[i];
                points[i] = points[j];
                points[j] = tmp;
            }
        }
        this->addVertex(points[i], true);
    }
    this->addVertex(points[i],true);
}

void VoroServer::GrahamScan(std::vector<Point> p) {
    std::vector<point2d> points;
    std::vector<point2d> convex_hull;

    for (unsigned int i=0; i<p.size();i++){
        point2d tmp_pnt;
        tmp_pnt.x = p.at(i).x();
        tmp_pnt.y = p.at(i).y();
        points.push_back(tmp_pnt);
    }

    GrahamScanConvexHull()(points, convex_hull);

    for(unsigned int i=0; i<convex_hull.size();i++){
        this->addVertex(Point(convex_hull.at(i).x,convex_hull.at(i).y),true);
    }
}

void VoroServer::vertsToVector(std::vector<Point> *v) {
    Vertex* pointer = this->cell.origin;

    while (pointer != NULL) {
        v->push_back(pointer->loc);
        pointer = pointer->next;
    }
}

//  Globals which should be set before calling this function:
//
//  int    polySides  =  how many corners the polygon has
//  float  polyX[]    =  horizontal coordinates of corners
//  float  polyY[]    =  vertical coordinates of corners
//  float  x, y       =  point to be tested
//
//  (Globals are used in this example for purposes of speed.  Change as
//  desired.)
//
//  The function will return YES if the point x,y is inside OR ON the polygon, or
//  NO if it is not.  If the point is exactly on the edge of the polygon,
//  then the function may return YES or NO.
//
//  Note that division by zero is avoided because the division is protected
//  by the "if" clause which surrounds it.

bool VoroServer::pointInPolygon(Point p) {
    int polySides = this->cell.n;
    float x = p.x();
    float y = p.y();
    float polyYi, polyYj;
    float polyXi, polyXj;
    int i, j = polySides-1;
    bool oddNodes = false;

    std::vector<Point> verts;
    vertsToVector(&verts);

//    verts =myUnique(verts);

    for (i=0; i<polySides; i++) {
        polyYi = verts[i].y();
        polyXi = verts[i].x();
        polyYj = verts[j].y();
        polyXj = verts[j].x();

        if ( ((polyYi>y) != (polyYj>y)) &&
             (x <= (polyXj-polyXi) * (y-polyYi) / (polyYj-polyYi) + polyXi) ) {
            oddNodes =! oddNodes;
        }
        j=i;
    }

    return oddNodes;
}

double VoroServer::calcArea(){
    double sum =0;
    std::vector<Point> verts;
    vertsToVector(&verts);
    unsigned int i,j = verts.size()-1;

    for(i=0;i<verts.size();i++){
        sum += (verts[j].x()+verts[i].x())*(verts[j].y()-verts[i].y());
        j=i;
    }
    return abs(sum/2);

    /*
     *
     *   double  area=0. ;
  int     i, j=points-1  ;

  for (i=0; i<points; i++) {
    area+=(X[j]+X[i])*(Y[j]-Y[i]); j=i; }

  return area*.5;
     */
}

// Takes an array of atleast 3 points and returns if they are ccw
// **Ref: Stackoverflow, Beta: Math - How to determine if a list of polygon points are in clockwise order?
// http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
bool ccw(Point p[], int n) {
    double sum = 0;

    // Sum over (x2-x1)(y2+y1)
    for (int i=0;i<n-1;i++) {
        sum += (p[i+1].x()-p[i].x())*(p[i+1].y()+p[i].y());
    }
    return sum <=0;
}

/*
 *  Determines if the tree points p1(a,b), p2(c,d) p3(e,f) are ccw, by looking at the slop of
 *  line (a,b)(c,d) vs line (a,b)(e,f)
 *  ccw <=> (f - b)(c - a)<(d - b)(e - a)
 *  **Ref: J. Erickson, “Lecture: Convex Hulls,” 2008.
 *  www.cs.uiuc.edu/~jeffe/teaching/compgeom/notes/01-convexhull.pdf
 */
bool ccw(Point p1, Point p2, Point p3) {
    return ((p3.y() - p1.y())*(p2.x() - p1.x())) < ((p2.y() - p1.y())*(p3.x() - p1.x()));
}

void insertIntoVec(std::vector<Point> *v, Point p) {
    Point tmp;
    for (unsigned int i=0;i<v->size();i++) {
        tmp = v->at(i);
        if (tmp.equal(p)) {
            return;
        }
    }

    v->push_back(p);
}
