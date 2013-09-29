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

#include "QuadServer.h"
#include <cstdio>
#include "Point.h"

/***************************************
 *  QuadTree
 **************************************/

QuadServer::QuadServer(){
    key = OverlayKey::ZERO;
    loc = Point(0,0);
    lvl = -1;           // Set lvl to -1 indicationg that it has not been assigned
    cell.n = 0;
    childCount = 0;
    parent = NULL;
    myClients.clear();
    neighbours.clear();
}

QuadServer::QuadServer(OverlayKey k){
    key = k;
    loc = Point(0,0);
    lvl = -1;           // Set lvl to -1 indicationg that it has not been assigned
    cell.n = 0;
    childCount = 0;
    parent = NULL;
    myClients.clear();
    neighbours.clear();
}

QuadServer::QuadServer(OverlayKey k, double x, double y)
{
    key = k;
    loc = Point(x,y);
    lvl = 0;
    cell.n = 0;
    childCount = 0;
    parent = NULL;
    myClients.clear();
    neighbours.clear();
}

QuadServer::~QuadServer() {
}

void QuadServer::setMasterKey(OverlayKey k) {
    this->masterKey = k;
}

bool QuadServer::isLoaded() {
    return this->myClients.size()>MAXCLIENTS;
}

bool QuadServer::underLoaded() {
    return (this->lvl !=-1 && this->myClients.size()<MINCLIENTS && this->cell.n==1);
}

/*
 *  Adds a new Rectangle, enforcing topLeft and botRight Rectangle format
 *  p1-------
 *  |       |
 *  |       |
 *  -------p2
 */
void QuadServer::addRect(Point p1, Point p2) {
    double tempx;
    double tempy;
    if (cell.n == 4) {
        return;
    }

    if (p1.x() > p2.x()) {
        tempx = p1.x();
        p1.setX(p2.x());
        p2.setX(tempx);
    }
    if (p1.y() > p2.y()) {
        tempy = p2.y();
        p2.setY(p1.y());
        p1.setY(tempy);
    }

    this->cell.rect.push_back(new Rectangle(p1,p2));
    cell.n++;
}

void QuadServer::addRect(Rectangle* r) {
    this->addRect(r->topLeft, r->botRight);
}

/*
 *  Devide the current rectangle into four and return true if successful.
 *  |  1  |  2  |
 *  -------------
 *  |  4  |  3  |
 *
 *  Can only devide upto level 2.
 */
bool QuadServer::devide() {

    if (this->lvl == 2) {
        return false;
    }
        // Get Rect
        Point p1 = (*cell.rect.begin())->topLeft;
        Point p2 = (*cell.rect.begin())->botRight;
        this->cell.n = 0;
        this->cell.rect.clear();

        // Devide into four rects and add to this.cell
        Point p3 = Point(p2.x(), p1.y());
        Point p4 = Point(p1.x(), p2.y());
        Point p5 = Point((p2.x() + p1.x())/2,(p2.y() + p1.y())/2);

        // Add all four rectangles
        this->addRect(p1,p5);
        this->addRect(p5,p3);
        this->addRect(p5,p2);
        this->addRect(p4,p5);

        // Set location in topLeft rect and increase lvl
        this->loc = Point((p5.x()+p1.x())/2, (p5.y()+p1.y())/2);
        this->lvl++;
        return true;
}

/*
 *  Determines if four rectangles can merge into one. Returns true if successful, else false
 */

bool QuadServer::merge() {
    std::list<Rectangle*>::iterator it;
    Rectangle* curRect;
    Rectangle* newR = (*cell.rect.begin());

    if (this->cell.n != 4) {
        return false;
    }

    // Find the rectangle that is the largest
    for (it = this->cell.rect.begin(); it != cell.rect.end();it++) {
        curRect = (*it);
        if (newR->topLeft.x() >= curRect->topLeft.x() && newR->topLeft.y() >= curRect->topLeft.y()) {
            newR->topLeft = curRect->topLeft;
        }
        if (newR->botRight.x() <= curRect->botRight.x() && newR->botRight.y() <= curRect->botRight.y()) {
            newR->botRight = curRect->botRight;
        }
    }

#ifdef _DEBUG
    if (this->master) {
        if (!(newR->topLeft.equal(Point(0,0)) && newR->botRight.equal(Point(500,500)))) {
            printf("Master bug");
        }
    }
#endif

    this->cell.n=0;
    this->cell.rect.clear();
    this->addRect(newR);
    this->lvl--;
    this->loc = Point((newR->topLeft.x()+newR->botRight.x())/2, (newR->topLeft.y()+newR->botRight.y())/2);
    return true;
}

/*
 *  Transfers the a Rectangle to t, returns true is successful.
 */

bool QuadServer::transfer(QuadServer *t) {
    if (this->cell.n == 1) {
        if (!this->devide()) {
            return false;
        }
    }

    Rectangle* curRect = (*this->cell.rect.rbegin());
    Point p1 = curRect->topLeft;
    Point p2 = curRect->botRight;

    this->cell.rect.pop_back();
    this->cell.n--;

    t->addRect(p1,p2);
    t->lvl = this->lvl;

    t->loc = Point((p1.x()+p2.x())/2, (p1.y()+p2.y())/2);

    t->parent = this;
    this->childCount++;

    // Update neighbour list
    this->neighbours.insert(t->key);
    t->neighbours.insert(this->key);


#ifdef _DEBUG
    if (this->neighbours.size() > 8) {
        this->printNeighbourLocs();
    }
#endif

    return true;
}

bool QuadServer::returnArea() {
    set <QuadServer*>::iterator it;
    set <Client*>::iterator cit;

    if (this->parent!=NULL && this->parent->lvl == this->lvl && this->childCount == 0 ) {
        Rectangle* curR = (*this->cell.rect.rbegin());

        // Remove self from parent
        this->parent->childCount--;
        this->parent->addRect(curR);

        this->parent->merge();


        // Transfer all myClients
        for (cit = this->myClients.begin(); cit != this->myClients.end();cit++) {
            this->parent->myClients.insert(*cit);
        }

        this->parent->checkOwnership();

        // Remove me from all neighbour lists
//        for(it = this->neighbours.begin(); it != this->neighbours.end(); it++) {
//            this->parent->addAdjacent(*it);
//            (*it)->neighbours.erase(this);
//        }

        // Set lvl be deleted
        this->lvl = -1;
        return true;
    }
    return false;
}

/*
 *  Determines id the test point tp is inside or on the border of the area owned
 */

bool QuadServer::insideArea(Point* tp) {
    std::list<Rectangle*>::iterator it;
    Rectangle* curRec;
    for (it = this->cell.rect.begin(); it != cell.rect.end();it++) {
        curRec = (*it);
        if (inRect(tp, curRec)) {
            return true;
        }
    }
    return false;
}

/*
 *  Tests if the QuadServer t is adjacent and adds to neigbour list
 *   a  |   b   |   c
 *      |       |
 *  ----p1-----p3----
 *  h   |   t   |   d
 *      |       |
 *  ----p4-----p2----
 *  g   |   f   |   e
 *      |       |
 */

bool QuadServer::adjacent(std::list<Rectangle*> *rects) {
    std::list<Rectangle*>::iterator rit;
    Rectangle* tRect;

    Point *p3, *p4;
    for (rit = rects->begin(); rit != rects->end(); rit++) {
        tRect = (*rit);
        p3 = new Point(tRect->botRight.x(),tRect->topLeft.y());
        p4 = new Point(tRect->topLeft.x(), tRect->botRight.y());

        if (this->insideArea(&tRect->topLeft) || this->insideArea(&tRect->botRight) ||
                this->insideArea(p3) || this->insideArea(p4)) {
            return true;
        }
    }
    return false;
}

bool QuadServer::ownership(Client* c) {
    if (this->insideArea(&c->loc)) {
        return true;
    }
    return false;
}


std::vector<Client*> QuadServer::checkOwnership() {
    set <Client*>::iterator it;
    set <Client*> tmp = myClients;
    std::vector<Client*> notMine;

    for (it = tmp.begin(); it != tmp.end(); ++it) {
        if (!this->ownership(*it)) {
            notMine.push_back(*it);
            this->myClients.erase(*it);
        }
    }
    return notMine;
}

void QuadServer::printNeighbourLocs(){
    set <OverlayKey>::iterator it;
    printf("My loc (%g,%g)\n",this->loc.x(),this->loc.y());
    for(it = this->neighbours.begin(); it != this->neighbours.end(); it++) {
        printf("N's loc %f\n",(*it).toDouble());
    }
}

bool inRect(Point* tp, Rectangle* r) {
    if (tp->x() >= r->topLeft.x() && tp->x() <= r->botRight.x()) {
        if (tp->y() >= r->topLeft.y() && tp->y() <= r->botRight.y()) {
            return true;
        }
    }
    return false;
}
