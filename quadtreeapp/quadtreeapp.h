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

#ifndef QUADTREEAPP_H_
#define QUADTREEAPP_H_

#include <omnetpp.h>
#include "BaseApp.h"

class quadtreeapp: public BaseApp {
    // Self timer messages
    cMessage *serverTimer;
    cMessage *clientAddTimer;
    cMessage *clientMoveTimer;

    // // application routines
    void initializeApp();                           // called when the module is being created
    void finishApp();                              // called when the module is about to be destroyed

    void handleTimerEvent(cMessage* msg);          // called when we received a timer message
    void deliver(OverlayKey& key, cMessage* msg);  // called when we receive a message from the overlay

public:
    quadtreeapp(){serverTimer = NULL;};
    ~quadtreeapp(){ cancelAndDelete(serverTimer);};
};

#endif /* QUADTREEAPP_H_ */
