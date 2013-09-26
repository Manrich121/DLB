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

#define WAIT 1


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

    srand(time(NULL));
    // Get params set in .ini file
    maxServers = par("largestKey");
    clientCount = 0;
    myKey = OverlayKey::ZERO;
    thisServer = NULL;

    //TODO: add WATCH on data vars to record
    WATCH(clientCount);
    WATCH(myKey);

    if (this->getParentModule()->getParentModule()->getIndex() == 0) {
        this->master = true;
        // start our timer!
        ticTimer = new cMessage("QuadTreeApp Timer");
        scheduleAt(simTime() + 1 + WAIT, ticTimer);

//        clientAddTimer = new cMessage("Add or Remove Client");
//        scheduleAt(simTime() + 2, clientAddTimer);

        // start client timer
        this->clientMoveTimer = new cMessage("ClientMove Timer");
        scheduleAt(simTime() + 0.5 + WAIT, clientMoveTimer);

        serverTimer = new cMessage("Server load check");
        scheduleAt(simTime() + 2 + WAIT, serverTimer);

        thisServer = new QuadServer(WIDTH/2,WIDTH/2);
        thisServer->addRect(Point(0,0), Point(WIDTH,WIDTH));
        addClient();
        addClient();
        addClient();
        addClient();
        addClient();
        sCount=1;


    }else{
        this->master = false;
        serverTimer = NULL;
        clientAddTimer = NULL;
        clientMoveTimer = NULL;
    }

    bindToPort(2000);
}

// finishApp is called when this module is being distroyed
void QuadtreeApp::finishApp(){
    cancelAndDelete(serverTimer);
    cancelAndDelete(clientMoveTimer);
    cancelAndDelete(clientAddTimer);
    cancelAndDelete(ticTimer);
    // TODO: globalStatistics->addStdDev("ManApplication: Sent packets", numSent);
}

// handleTimerEvent() is called when a self message is received
void QuadtreeApp::handleTimerEvent(cMessage* msg){
    myKey = this->overlay->getThisNode().getKey();
    // is this Tic timer?
    if (msg == ticTimer) {
        scheduleAt(simTime() + 1, ticTimer);

        NodeVector* neighs = this->overlay->neighborSet(maxServers);
        OverlayKey randomKey = neighs->at((intuniform(1, neighs->size()-1))).getKey();
        EV << "QuadtreeApp::handelTimerEvent =>  Chosen randomKey " << randomKey << std::endl;

        DLBMessage *myMessage; // the message we'll send
        myMessage = new DLBMessage();
        myMessage->setType(LOC_MSG); // set the message type to LOC_MSG
        myMessage->setSenderKey(myKey);  // Store this
        myMessage->setByteLength(100); // set the message length to 100 bytes

        EV << "QuadtreeApp::handelTimerEvent => " << thisNode.getIp() << ": Sending packet to "
           << randomKey << "!" << std::endl;

        callRoute(randomKey, myMessage); // send it to the overlay
        addClient();
    }else {
    /*
     *  Check load of a server
     */
        if (msg == serverTimer) {
            scheduleAt(simTime() + 2, serverTimer);
            if (thisServer != NULL){
                checkLoad();
            }
        }else{
            /*
             * Add or Remove a client from this server
             */

            if (msg == clientAddTimer) {
                scheduleAt(simTime() + 2, clientAddTimer);
                if (thisServer != NULL){
                    double r = rand()%100;
                    if (r<50){
                        addClient();
                    }else{
    //                  Remove client
                        removeClient();
                    }
                }
            }else{
                /*
                 * Update thisServer's client movement and check ownership
                 */
                if (msg == clientMoveTimer) {
                    scheduleAt(simTime() + 0.5, clientMoveTimer);
                    if (thisServer != NULL) {
                        if (thisServer->myClients.size() > 0) {
//                            EV << "thisServer.myClients.size()" << thisServer->myClients.size() << std::endl;
                            clientUpdate();
                        }
                    }
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
     myKey = this->overlay->getThisNode().getKey();
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
        myMessage->setType(DEBUG_MSG); // set the message type to LOC_MSG
        myMessage->setSenderKey(myKey);  // Store this
        myMessage->setByteLength(100); // set the message length to 100 bytes


        EV << "QuadtreeApp::deliver => " << thisNode.getIp() << ": Got packet from "
           << senderKey << ", sending back!"
           << std::endl;

        callRoute(senderKey,myMsg);
    }else{
        if (myMsg->getType() == DEBUG_MSG){
            EV << "QuadtreeApp::deliver => " << thisNode.getIp() << ": Got reply from "<< myMsg->getSenderKey() << std::endl;
        }else{
            if (myMsg->getType() == SERVER_MSG) {
                // Create new QuadServer object and assign values to it
                thisServer = new QuadServer();
                *thisServer = myMsg->getTransferServer();
                this->clientCount = thisServer->myClients.size();
                EV << "+++++++++++++++++\nQuadtreeApp::deliver => " << myKey << " Got my new server state" << std::endl;
                EV << "Key: " << thisServer->key << "\n"
                     << "Loc : (" << thisServer->loc.x() << "," << thisServer->loc.y() << ")\n"
                     << "Neighbours size : " << thisServer->neighbours.size() << "\n"
                     << "myClients.size : " << thisServer->myClients.size()
                     << "\n+++++++++++++++" << std::endl;

                // start client timer
                this->clientMoveTimer = new cMessage("ClientMove Timer");
                scheduleAt(simTime() + 0.5, clientMoveTimer);

            }else{
                if (myMsg->getType() == CLIENTRANS_MSG) {
                    EV << "------------------\nQuadtreeApp::deliver => Transfered client\n" << std::endl;
//                  Check ownership on new clients and insert into thisServer.myClients
                    std::vector<Client*> notMine = myMsg->getClients();
                    std::vector<Client*>::iterator it;
                    for(unsigned int i=0;i<notMine.size();i++){
                        if(thisServer->ownership(notMine.at(i))){
                            thisServer->myClients.insert(notMine.at(i));
                            EV << "QuadtreeApp::deliver => Client (" << notMine.at(i)->loc.x() << "," << notMine.at(i)->loc.y() << ") added" << std::endl;
                        }
                    }
                    this->clientCount = thisServer->myClients.size();
                    EV << "------------------" << std::endl;
                }
            }
        }
        delete msg;
    }
}

void QuadtreeApp::addClient() {
    thisServer->myClients.insert(new Client(thisServer->loc, WIDTH));
    clientCount++;
    EV << "##############\nQuadtreeApp::addClient => " << thisNode.getIp() << " Added new Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y() << std::endl;
    EV << "##############"<< std::endl;
}

void QuadtreeApp::removeClient() {
    thisServer->myClients.erase(*thisServer->myClients.rbegin());
    clientCount--;
}

void QuadtreeApp::clientUpdate() {
    set <Client*>::iterator it;
    set <Client*>::iterator tmp;
    std::vector<Client*> notMine;
    EV << "\n------------ "<< thisNode.getIp()<< " Update Clients--------------" << std::endl;
    for (it = thisServer->myClients.begin(); it != thisServer->myClients.end();it++) {
       (*it)->move();
       EV << "QuadTreeApp::clientUpdate: " << (*it)->loc.x() << ", " << (*it)->loc.y() << std::endl;
       if (!thisServer->ownership(*it)){
           EV << "QuadTreeApp::clientUpdate: REMOVED CLIENT: " << (*it)->loc.x() << ", " << (*it)->loc.y() << std::endl;
           notMine.push_back(*it);
//           thisServer->myClients.erase(*it);
//           clientCount--;
       }
    }
    EV << "------------ Update Clients--------------" << std::endl;
    EV << "notMine.size() : " << notMine.size() << std::endl;

    thisServer->checkOwnership();

    if (notMine.size() > 0) {
        EV << "\n------------Transfer Clients--------------\n\n" << std::endl;
        // Transfer clients to all my neighbours
        set <QuadServer*>::iterator sit;
        DLBMessage* clientTMsg = new DLBMessage();
        clientTMsg->setType(CLIENTRANS_MSG);
        clientTMsg->setSenderKey(myKey);
        clientTMsg->setClients(notMine);
        clientTMsg->setByteLength(100); // set the message length to 100 bytes
        for(sit = thisServer->neighbours.begin(); sit != thisServer->neighbours.end(); sit++) {
            DLBMessage* dupmsg;
            dupmsg = clientTMsg->dup();
            EV << "\nQuadTreeApp::clientUpdate => Transfer clients to " << (*sit)->key << "\n" << std::endl;
            callRoute((*sit)->key,dupmsg);
        }
        EV << "\n------------ Transfer Clients--------------\n" << std::endl;
    }

}

void QuadtreeApp::checkLoad() {
    if (thisServer->isLoaded() && sCount < maxServers){
        // Select new server key
        EV << "+++++++++++++++++ checkLoad ++++++++++++++++++" << std::endl;
        OverlayKey newKey =  getNewServerKey();
        EV << "QuadtreeApp::checkload => chosen key: " << newKey << std::endl;
        if (newKey.isUnspecified())
            return;
        QuadServer* newServer = new QuadServer(newKey); // Init a new server object with key=newKey
        sCount++;
        if (thisServer->transfer(newServer)) {
            clientCount = thisServer->myClients.size();
            DLBMessage *myMessage; // the message we'll send
            myMessage = new DLBMessage();
            myMessage->setType(SERVER_MSG); // set the message type to LOC_MSG
            myMessage->setSenderKey(myKey);  // Store this
            myMessage->setByteLength(100); // set the message length to 100 bytes
            myMessage->setTransferServer(*newServer);

            EV << "QuadtreeApp::checkLoad => Overloaded and setting up new server, key: " << newKey << " MyLoc (" << thisServer->loc.x() << "," << thisServer->loc.y()
                    << ") NewServloc (" << newServer->loc.x() << "," << newServer->loc.y() << ")"<< std::endl;
            callRoute(newKey, myMessage);
        }
        EV << "+++++++++++++++++ checkLoad ++++++++++++++++++" << std::endl;
    }

}

OverlayKey QuadtreeApp::getNewServerKey() {
    std::vector<NodeHandle>::const_iterator it;
    std::set<OverlayKey>::iterator kit;
    NodeVector* neighs = this->overlay->neighborSet(maxServers);

    EV << "QuadtreeApp::checkLoad =>My " << thisNode.getIp() << " neighbours size: " << neighs->size() << std::endl;

    for (it = neighs->begin(); it != neighs->end(); it++) {
        kit = inUse.find((*it).getKey());
        if (kit == inUse.end() && (*it).getKey() != myKey) {
            inUse.insert((*it).getKey());
            EV << "QuadtreeApp::checkLoad => My neighbour: " << (*it).getKey() << " Ip: " << (*it).getIp() << std::endl;
            return (*it).getKey();
        }
    }
    return OverlayKey::UNSPECIFIED_KEY;
}


