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

#define MASTERKEY 777


class QuadtreeApp: public BaseApp {

    //Stats signals
    simsignal_t overloadSig;
    simsignal_t inUseSig;
    simsignal_t freeServSig;

    simsignal_t neighSig;
    simsignal_t controlSig;
    simsignal_t clientTransSig;

    simsignal_t clientOwn;
    simsignal_t clientD;
    simsignal_t calcSig;

    cOutVector globClientDens;

    // Self timer messages
    cMessage *ticTimer;
    cMessage *serverTimer;
    cMessage *clientAddTimer;
    cMessage *clientMoveTimer;
    cMessage *setupMessage;

    double clientDens;

    //params
    int maxServers;
    int maxClients;
    int globClientCount;
    int clientCount;
    int areaDim;
    double leaveChance;
    double clientPeriod;
    double loadPeriod;
    int neighCount;
    double area;
    bool master;
    OverlayKey myKey;
    QuadServer* thisServer;

    // Master server params
    int sCount;
    int allKeysCount;
    std::set<OverlayKey> inUse;
    std::set<OverlayKey> overloadSet;
    set<OverlayKey> allKeys;

    // application routines
    void initializeApp(int stage);                 // called when the module is being created
    void finishApp();                              // called when the module is about to be destroyed
    void handleTimerEvent(cMessage* msg);          // called when we received a timer message
    void deliver(OverlayKey& key, cMessage* msg);  // called when we receive a message from the overlay

    void addClient();
    void removeClient();
    void clientUpdate();
    void checkLoad();
    OverlayKey getNewServerKey(OverlayKey key);
    void sendNewServer(OverlayKey newKey);
    void returnServer(QuadServer* retServer);
    void updateNeighbours();


public:
    QuadtreeApp(){ticTimer = NULL;
    };
    ~QuadtreeApp(){ cancelAndDelete(ticTimer);};
};

#endif /* QUADTREEAPP_H_ */
