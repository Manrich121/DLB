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

#include <string>
#include "UnderlayConfigurator.h"
#include "GlobalStatistics.h"
#include "quadtreeapp.h"
#include "NeighborCache.h"

#include "DLBMessage_m.h"


// This line tells the simulator that QuadtreeApp is going to be extended using C++ code.
// It *must* be present (only once) somewhere in your code, outside a function, for each module you'll be extending.
Define_Module(QuadtreeApp);

// initializeApp() is called when the module is being created.
// Use this function instead of the constructor for initializing variables.
void QuadtreeApp::initializeApp(int stage)
{
    // initializeApp will be called twice, each with a different stage.
    // stage can be either MIN_STAGE_APP (this module is being created),
    // or MAX_STAGE_APP (all modules were created).
    // We only care about MIN_STAGE_APP here.

    if (stage != MIN_STAGE_APP) return;

    // Get params set in .ini file
    maxServers = par("largestKey");
    clientCount = 0;

    //TODO: add WATCH on data vars to record
    WATCH(clientCount);

    if (this->getParentModule()->getParentModule()->getIndex() == 0) {
        this->master = true;
        // start our timer!
        ticTimer = new cMessage("QuadTreeApp Timer");
        scheduleAt(simTime() + 1, ticTimer);

        clientMoveTimer = new cMessage("ClientMove Timer");
        scheduleAt(simTime() + 0.5, clientMoveTimer);

        serverTimer = new cMessage("Server load check");
        scheduleAt(simTime() + 2, serverTimer);

        thisServer = new QuadServer(WIDTH/2,WIDTH/2);
        thisServer->addRect(Point(0,0), Point(WIDTH,WIDTH));
        addClient();

    }else{
        this->master = false;
    }

    bindToPort(2000);
}

// finishApp is called when this module is being distroyed
void QuadtreeApp::finishApp(){
    // TODO: globalStatistics->addStdDev("ManApplication: Sent packets", numSent);
}

// handleTimerEvent() is called when a self message is received
void QuadtreeApp::handleTimerEvent(cMessage* msg){

        // is this Tic timer?
    if (msg == ticTimer) {
        scheduleAt(simTime() + 1, ticTimer);

        NodeVector* neighs = this->overlay->neighborSet(maxServers);
        OverlayKey randomKey = neighs->at((intuniform(1, neighs->size()-1))).getKey();
        EV << "QuadtreeApp::handelTimerEvent =>  Chosen randomKey " << randomKey << std::endl;

        DLBMessage *myMessage; // the message we'll send
        myMessage = new DLBMessage();
        myMessage->setType(LOC_MSG); // set the message type to LOC_MSG
        myMessage->setSenderKey(this->overlay->getThisNode().getKey());  // Store this
        myMessage->setByteLength(100); // set the message length to 100 bytes

        EV << "QuadtreeApp::handelTimerEvent => " << thisNode.getIp() << ": Sending packet to "
           << randomKey << "!" << std::endl;

        callRoute(randomKey, myMessage); // send it to the overlay
    }else {
        if (msg == serverTimer) {
            checkLoad();
        }else{
            if (msg == clientAddTimer) {
                addClient();

            }else{
                if (msg == clientMoveTimer) {
                    clientUpdate();

                }else{
                    // Unknown messages can be deleted
                    cancelAndDelete(msg);
                }
            }
        }
    }
}

// deliver() is called when a message is received from the overlay
// Unknown packets can be safely deleted here.
void QuadtreeApp::deliver(OverlayKey& key, cMessage* msg) {
    // we are only expecting messages of type DLBMessage, throw away any other
    DLBMessage *myMsg = dynamic_cast<DLBMessage*>(msg);
    if (myMsg == NULL) {
        delete msg; // type unknown!
        return;
    }

    if (myMsg->getType() == LOC_MSG){

        OverlayKey senderKey = myMsg->getSenderKey();
        delete myMsg;
        DLBMessage *myMessage; // the message we'll send
        myMessage = new DLBMessage();
        myMessage->setType(LOAD_MSG); // set the message type to LOC_MSG
        myMessage->setSenderKey(this->overlay->getThisNode().getKey());  // Store this
        myMessage->setByteLength(100); // set the message length to 100 bytes


        EV << "QuadtreeApp::deliver => " << thisNode.getIp() << ": Got packet from "
           << senderKey << ", sending back!"
           << std::endl;

        callRoute(senderKey,myMsg);
    }else{
        if (myMsg->getType() == LOAD_MSG){
            EV << "QuadtreeApp::deliver => " << thisNode.getIp() << ": Got reply from "<< myMsg->getSenderKey() << std::endl;
        }
        // Delete messages unrecognized or not used anymore
        delete msg;
    }
}

void QuadtreeApp::addClient() {
    thisServer->myClients.insert(new Client(thisServer->loc, WIDTH));
    clientCount++;
    EV << "QuadtreeApp::addClient => Added new Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y() << std::endl;
}

void QuadtreeApp::removeClient() {
    thisServer->myClients.erase(*thisServer->myClients.rbegin());
    clientCount--;
}

void QuadtreeApp::clientUpdate() {
    set <Client*>::iterator it;
    set <Client*>::iterator tmp;

    for (it = thisServer->myClients.begin(); it != thisServer->myClients.end();it++) {
       (*it)->move();
//       EV << "QuadTreeApp::clientUpdate: " << (*it)->loc.x() << ", " << (*it)->loc.y() << std::endl;
    }

    thisServer->checkOwnership();
}

void QuadtreeApp::checkLoad() {

    NodeVector* neighs = this->overlay->neighborSet(maxServers);

    std::vector<NodeHandle>::const_iterator it;
    EV << "QuadtreeApp::checkLoad =>My " << thisNode.getIp() << " neighbours size: " << neighs->size() << std::endl;

    for (it = neighs->begin(); it != neighs->end(); it++) {
        EV << "QuadtreeApp::checkLoad => My neighbour: " << (*it).getKey() << " Ip: " << (*it).getIp() << std::endl;
    }
//    vector <Server*>::iterator sit;
//    Server* curServ;
//
//    for (sit = this->servers.begin(); sit != this->servers.end(); sit++) {
//        curServ = (*sit);
//
//        if (curServ->underLoaded()) {
//            if (curServ->returnArea()) {
//                servers.erase(sit);
//                break;
//            }
//        }
//
//        if (curServ->isLoaded() && servers.size() <16) {
//            Server* newServ = new Server();
//            if (curServ->transfer(newServ)) {
//                servers.push_back(newServ);
//                break;
//            }
//        }
//    }
}

