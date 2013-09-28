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

    srand(time(NULL));
    // Get params set in .ini file
    maxServers = par("largestKey");
    clientCount = 0;
    myKey = OverlayKey::ZERO;
    thisServer = NULL;

    //TODO: add WATCH on data vars to record
    WATCH(sCount);
    WATCH(clientCount);
    WATCH(myKey);
    if (this->getParentModule()->getParentModule()->getIndex() == 0) {
        this->master = true;
    }else{
        this->master = false;
    }

    setupMessage = new cMessage("Setup");
    scheduleAt(simTime() + maxServers*2, setupMessage);

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
    // is this Tic timer?
    if (msg == ticTimer) {
        scheduleAt(simTime() + 2, ticTimer);

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
//        addClient();
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
                scheduleAt(simTime() + 1, clientAddTimer);
                if (thisServer != NULL){
                    double r = rand()%100;
                    if (r>20){
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
                    scheduleAt(simTime() + 0.2, clientMoveTimer);
                    if (thisServer != NULL) {
                        if (thisServer->myClients.size() > 0) {
//                            EV << "thisServer.myClients.size()" << thisServer->myClients.size() << std::endl;
                            clientUpdate();
                        }
                    }
                }else{
                    if (msg == setupMessage){
                        myKey = this->overlay->getThisNode().getKey();
                        sCount = 0;
                        if(master){
                            clientAddTimer = new cMessage("Client: Add or Remove");
                            scheduleAt(simTime() + 1, clientAddTimer);

                            // start client timer
                            this->clientMoveTimer = new cMessage("ClientMove Timer");
                            scheduleAt(simTime() + 0.2, clientMoveTimer);

                            serverTimer = new cMessage("Server load check");
                            scheduleAt(simTime() + 2, serverTimer);

                            thisServer = new QuadServer(myKey, WIDTH/2,WIDTH/2);
                            thisServer->setMasterKey(myKey);
                            thisServer->addRect(Point(0,0), Point(WIDTH,WIDTH));
                            addClient();
                            addClient();
                            addClient();
                            addClient();
                            addClient();
                            sCount=1;
                        }
                        cancelAndDelete(msg);
                    }else{
                        // Unknown messages can be deleted
                        cancelAndDelete(msg);
                    }
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

    switch (myMsg->getType()){
    case LOC_MSG : {
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
    }break;
    case DEBUG_MSG : {
            EV << "QuadtreeApp::deliver => " << thisNode.getIp() << ": Got reply from "<< myMsg->getSenderKey() << std::endl;
        }delete msg; break;
    case SERVER_MSG : {
        // Create new QuadServer object and assign values to it
        thisServer = new QuadServer;
        *thisServer = myMsg->getTransferServer();
        this->clientCount = thisServer->myClients.size();
        EV << "+++++++++++++++++\nQuadtreeApp::deliver => " << myKey << " Got my new server state" << std::endl;
        EV << "Loc : (" << thisServer->loc.x() << "," << thisServer->loc.y() << ")\n"
             << "Key : " << thisServer->key << "\n"
             << "Neighbours size : " << thisServer->neighbours.size() << "\n"
             << "myClients.size : " << thisServer->myClients.size() << "\n"
             << "Rec: tl(" << thisServer->cell.rect.front()->topLeft.x() << "," << thisServer->cell.rect.front()->topLeft.y() << ")"
             << "br(" << thisServer->cell.rect.front()->botRight.x() << "," << thisServer->cell.rect.front()->botRight.y() << ")"
             << "\n+++++++++++++++" << std::endl;

        // start client timer
        clientMoveTimer = new cMessage("ClientMove Timer");
        scheduleAt(simTime() + 0.5, clientMoveTimer);

        // start load Check timer
        serverTimer = new cMessage("Server load check");
        scheduleAt(simTime() + 2, serverTimer);

        }delete msg; break;
    case CLIENTRANS_MSG: {
        EV << "------------------ Client Transfer ----------------" << std::endl;
//                  Check ownership on new clients and insert into thisServer.myClients
        std::vector<Client*> notMine = myMsg->getClients();
        std::vector<Client*>::iterator it;
        for(unsigned int i=0;i<notMine.size();i++){
            EV << "QuadtreeApp::deliver => Client (" << notMine.at(i)->loc.x() << "," << notMine.at(i)->loc.y() << ") Mine? " <<  thisServer->ownership(notMine.at(i))  << std::endl;
            if(thisServer->ownership(notMine.at(i))){
                thisServer->myClients.insert(notMine.at(i));
            }
        }
        this->clientCount = thisServer->myClients.size();
        EV << "------------------ Client Transfer ----------------" << std::endl;
        }delete msg; break;
    case REQKEY_MSG: {
        if (master){
            OverlayKey slaveKey = myMsg->getSenderKey();
            delete myMsg;
            OverlayKey newKey = this->getNewServerKey();
            if (!newKey.isUnspecified()) {
                DLBMessage *myMessage; // the message we'll send
                myMessage = new DLBMessage();
                myMessage->setType(REQKEY_MSG); // set the message type
                myMessage->setSenderKey(newKey);  // Store the new serverkey in senderkey
                myMessage->setByteLength(sizeof(newKey)); // set the message length to size of an Overlaykey

                callRoute(slaveKey, myMessage);
            }
        }else{
            EV << "@@@@@@@@@@@@@@@@@@" << std::endl;
            OverlayKey newKey = myMsg->getSenderKey();
            sendNewServer(newKey);
            delete msg;
        }
        }break;
    case RETSERV_MSG: {
        // Create new QuadServer object and assign values to it
        QuadServer retServer = myMsg->getTransferServer();
        EV << "**************\nQuadtreeApp::deliver => " << myKey << " Got return server" << std::endl;
        EV << "Parent key : " << retServer.parent->key << std::endl;

        returnServer(&retServer);

        DLBMessage* freeMsg = new DLBMessage();
        freeMsg->setType(FREEME_MSG);
        freeMsg->setSenderKey(retServer.key);
        freeMsg->setByteLength(sizeof(retServer.key));

        callRoute(thisServer->masterKey, freeMsg);
        EV << "**************" << std::endl;

    }delete msg; break;
    case FREEME_MSG: {
        // Test to see if I am master
        if(master){
            inUse.erase(myMsg->getSenderKey());
            sCount--;
            EV << "**************\n Erased inUse["<<myMsg->getSenderKey() << "]" << std::endl;
        }
    }delete msg; break;
    }
}

void QuadtreeApp::addClient() {
        thisServer->myClients.insert(new Client(thisServer->loc, WIDTH));
        clientCount++;
        EV << "##############QuadtreeApp::addClient => " << thisNode.getIp() << " Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y();
        EV << "##############"<< std::endl;
}

void QuadtreeApp::removeClient() {
    if (thisServer->myClients.size() > 0){
        EV << "##############QuadtreeApp::removeClient => " << thisNode.getIp() << " Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y();
        EV << "##############"<< std::endl;
//        delete *thisServer->myClients.rbegin();
        thisServer->myClients.erase(*thisServer->myClients.rbegin());
        clientCount--;
    }
}

void QuadtreeApp::clientUpdate() {
    set <Client*>::iterator it;
    set <Client*>::iterator tmp;
    std::vector<Client*> notMine;

    EV << "------------ "<< thisNode.getIp()<< " Update " << thisServer->myClients.size() << " Clients--------------" << std::endl;
    for (it = thisServer->myClients.begin(); it != thisServer->myClients.end();it++) {
       (*it)->move();
       if (!thisServer->ownership(*it)){
           EV << "QuadTreeApp::clientUpdate: Transfer client: " << (*it)->loc.x() << ", " << (*it)->loc.y() << std::endl;
           notMine.push_back(*it);
           thisServer->myClients.erase(*it);
           clientCount--;
       }
    }

    if (notMine.size() > 0) {
        EV << "------------Transfer Clients--------------" << std::endl;
        EV << "notMine.size() : " << notMine.size() << "\n"
           << "neighbours.size() : " << thisServer->neighbours.size() << std::endl;

        // Transfer clients to all my neighbours
        set <QuadServer*>::iterator sit;
        DLBMessage* clientTMsg = new DLBMessage();
        clientTMsg->setType(CLIENTRANS_MSG);
        clientTMsg->setSenderKey(myKey);
        clientTMsg->setClients(notMine);
        clientTMsg->setByteLength(sizeof(notMine)); // set the message length to the size of the vector notMine

        for(sit = thisServer->neighbours.begin(); sit != thisServer->neighbours.end(); sit++) {
            DLBMessage* dupmsg;
            dupmsg = clientTMsg->dup();
            EV << "QuadTreeApp::clientUpdate => Transfer clients to " << (*sit)->key << std::endl;
            callRoute((*sit)->key,dupmsg);
        }
        delete clientTMsg;
        EV << "------------ Transfer Clients--------------" << std::endl;
    }
}

void QuadtreeApp::checkLoad() {
    if (thisServer->isLoaded()){
        if (master) {
            // Select new server key
            OverlayKey newKey =  getNewServerKey();
            if (newKey.isUnspecified())
                return;
            sendNewServer(newKey);
        }else{
            EV << "@@@@@@@@@@@@@@@@@@ Slave overload @@@@@@@@@@@@@@@@" << std::endl;
            EV << "QuadTree::checkLoad => slave overloaded, requesting new server" << std::endl;
            DLBMessage* reqMsg = new DLBMessage();
            reqMsg->setType(REQKEY_MSG); // set the message type
            reqMsg->setSenderKey(myKey); // Set senderKey to myKey
            reqMsg->setByteLength(100); // set the message length to 100 bytes

            callRoute(thisServer->masterKey, reqMsg);
            EV << "@@@@@@@@@@@@@@@@@@ Slave overload @@@@@@@@@@@@@@@@" << std::endl;
        }
    }else{
        if(thisServer->underLoaded()) {
            if (thisServer->parent != NULL && thisServer->parent->lvl == thisServer->lvl && thisServer->childCount == 0) {
                EV << "******************** Underload **********************" << std::endl;
                EV << "QuadtreeApp::checkLoad " << thisNode.getIp() << " underload transfering area and clients to parent (if possible)" << std::endl;

                DLBMessage* sretMsg = new DLBMessage();
                sretMsg->setType(RETSERV_MSG);
                sretMsg->setSenderKey(myKey);
                sretMsg->setTransferServer(*thisServer);
                sretMsg->setByteLength(sizeof(*thisServer));

                callRoute(thisServer->parent->key, sretMsg);

//                delete thisServer;
//                thisServer = NULL;
                clientCount = 0;
                //Cancel all timers
                cancelAndDelete(serverTimer);
                cancelAndDelete(clientMoveTimer);
                cancelAndDelete(clientAddTimer);
                cancelAndDelete(ticTimer);
                EV << "Cancel all timers" << std::endl;
                EV << "******************** Underload **********************" << std::endl;

            }
        }
    }
}

OverlayKey QuadtreeApp::getNewServerKey() {
    std::vector<NodeHandle>::const_iterator it;
    std::set<OverlayKey>::iterator kit;
    NodeVector* neighs = this->overlay->neighborSet(maxServers);

    if (sCount < maxServers) {
        EV << "QuadtreeApp::checkLoad =>My " << thisNode.getIp() << " OverlayNeighbours size: " << neighs->size()-1 << std::endl;

        for (it = neighs->begin(); it != neighs->end(); it++) {
            kit = inUse.find((*it).getKey());
            if (kit == inUse.end() && (*it).getKey() != myKey) {
                inUse.insert((*it).getKey());
                EV << "QuadtreeApp::checkLoad => My neighbour: " << (*it).getKey() << " Ip: " << (*it).getIp() << std::endl;
                sCount++;
                return (*it).getKey();
            }
        }
    }
    return OverlayKey::UNSPECIFIED_KEY;
}

void QuadtreeApp::sendNewServer( OverlayKey newKey) {
    EV << "+++++++++++++++++ " << thisNode.getIp() << " SendNewServer ++++++++++++++++++" << std::endl;
    EV << "QuadtreeApp::sendNewServer to key: " << newKey << std::endl;
    QuadServer* newServer = new QuadServer(newKey); // Init a new server object with key=newKey
    newServer->setMasterKey(thisServer->masterKey); // Set master key used to request new server when loaded
    if (thisServer->transfer(newServer)) {
        clientCount = thisServer->myClients.size();
        DLBMessage *myMessage; // the message we'll send
        myMessage = new DLBMessage();
        myMessage->setType(SERVER_MSG); // set the message type to LOC_MSG
        myMessage->setSenderKey(myKey);  // Store this
        myMessage->setByteLength(sizeof(*newServer)); // set the message length to 100 bytes
        myMessage->setTransferServer(*newServer);

        EV << "QuadtreeApp::checkLoad => Overloaded and setting up new server, key: " << newKey << " MyLoc (" << thisServer->loc.x() << "," << thisServer->loc.y()
                << ") NewServloc (" << newServer->loc.x() << "," << newServer->loc.y() << ")"<< std::endl;
        callRoute(newKey, myMessage);
    }
    EV << "+++++++++++++++++ SendNewServer ++++++++++++++++++" << std::endl;
}

void QuadtreeApp::returnServer(QuadServer* retServer) {

    set <Client*>::iterator cit;
    set <QuadServer*>::iterator it;

    EV << "thisServer stuff" << std::endl;
    thisServer->childCount--;
    thisServer->addRect(*retServer->cell.rect.rbegin());
    thisServer->merge();

    EV << "Transfer all myClients" << std::endl;
    // Transfer all myClients
    for (cit = retServer->myClients.begin(); cit != retServer->myClients.end();cit++) {
        thisServer->myClients.insert(*cit);
    }
    this->clientCount = thisServer->myClients.size();

    EV << "Update neigbours" << std::endl;
    // Remove me from all neighbour lists
    for(it = retServer->neighbours.begin(); it != retServer->neighbours.end(); it++) {
        // TODO: Handle neighbours
//        thisServer->addAdjacent(*it);
//            (*it)->neighbours.erase(this);
    }

    retServer->lvl = -1;

    EV << "myClients.size() : " << retServer->myClients.size() << std::endl;
    EV << "myNeighbours.size() : " << retServer->neighbours.size() << std::endl;
    EV << "Lvl : " << retServer->lvl << std::endl;

}


