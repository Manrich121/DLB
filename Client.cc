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

#include "Client.h"

#include "stdlib.h"

Client::Client() {
    loc = Point();
}
Client::~Client() {
}

Client::Client(Point p, int max) {
    loc = p;
    vx = rand()%3 +1;
    vy = rand()%3 +1;
    edge = max;
}

void Client::move() {
    double x = loc.x()+vx;
    double y = loc.y()+vy;
    if (x>edge-1) {
        x = edge-1;
    }
    if(x<0) {
        x=0;
    }

    if (y>edge-1) {
        y = edge-1;
    }
    if(y<0) {
        y=0;
    }

    loc.setX(x);
    loc.setY(y);

    if (loc.x() >= edge-1 || loc.x() <= 0 ) {
        vx = -vx;
    }

    if (loc.y() >= edge-1 || loc.y() <= 0) {
        vy = -vy;
    }
}
