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
#include "QuadServer.h"

class QuadtreeApp: public BaseApp {

    //Stats signals
    simsignal_t msgCountSig;

    // Self timer messages
    cMessage *ticTimer;
    cMessage *serverTimer;
    cMessage *clientAddTimer;
    cMessage *clientMoveTimer;
    cMessage *setupMessage;

    //params
    int maxServers;
    int clientCount;
    int neighCount;
    bool master;
    OverlayKey myKey;
    QuadServer* thisServer;

    // Master server params
    int sCount;
    std::set<OverlayKey> inUse;

    // application routines
    void initializeApp(int stage);                 // called when the module is being created
    void finishApp();                              // called when the module is about to be destroyed
    void handleTimerEvent(cMessage* msg);          // called when we received a timer message
    void deliver(OverlayKey& key, cMessage* msg);  // called when we receive a message from the overlay

    void addClient();
    void removeClient();
    void clientUpdate();
    void checkLoad();
    OverlayKey getNewServerKey();
    void sendNewServer(OverlayKey newKey);
    void returnServer(QuadServer* retServer);
    void updateNeighbours();


public:
    QuadtreeApp(){ticTimer = NULL;
    };
    ~QuadtreeApp(){ cancelAndDelete(ticTimer);};
};

#endif /* QUADTREEAPP_H_ */
