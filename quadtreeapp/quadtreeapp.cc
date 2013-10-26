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
    maxServers = par("maxServers");
    maxClients = par("maxClients");
    clientPeriod = par("clientPeriod");
    loadPeriod = par("loadPeriod");
    leaveChance = par("leaveChance");
    areaDim = par("areaDim");
    globClientCount = 0;
    clientCount = 0;
    neighCount = 0;
    area=0;
    myKey = OverlayKey::ZERO;
    thisServer = NULL;

    //Stat counter
    numNeighMsg =0;
    numControlMsg =0;
    numClientTrans =0;

    clientDens =0;
    allKeysCount =0;

    //TODO: add WATCH on data vars to record
    WATCH(sCount);
    WATCH(allKeysCount);
    WATCH(clientCount);
    WATCH(globClientCount);
    WATCH(area);
    WATCH(clientDens);
    WATCH(neighCount);
    WATCH(myKey);


    // Register signals
    overloadSig = registerSignal("numOverloadServ");
    inUseSig = registerSignal("numInUseServ");
    freeServSig = registerSignal("numFreeServ");

    neighSig = registerSignal("numNeighMsg");
    controlSig = registerSignal("numControlMsg");
    clientTransSig = registerSignal("numClients");

    clientOwn = registerSignal("numClientOwn");
    clientD = registerSignal("clientDens");
    calcSig = registerSignal("numCalc");

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
    if(master)
        recordScalar("numCalc",QuadServer::numCalcs);
}

// handleTimerEvent() is called when a self message is received
void QuadtreeApp::handleTimerEvent(cMessage* msg){
    // is this Tic timer?
    if (msg == ticTimer) {
        scheduleAt(simTime() + 1, ticTimer);
        if(master){
            emit(inUseSig, inUse.size());
            emit(overloadSig, overloadSet.size());
            emit(freeServSig, maxServers-inUse.size());
        }

        area = thisServer->calcArea();
        clientDens = 2000*2000*thisServer->myClients.size()/area;
        emit(clientD,clientDens);
    }else {
    /*
     *  Check load of a server
     */
        if (msg == serverTimer) {
            scheduleAt(simTime() + loadPeriod, serverTimer);
            if (thisServer != NULL){
                checkLoad();
            }
        }else{
            /*
             * Add or Remove a client from this server
             */

            if (msg == clientAddTimer) {
                scheduleAt(simTime() + clientPeriod, clientAddTimer);
                if (thisServer != NULL){
                    double r = uniform(0,1);
                    if (r > leaveChance){
                        addClient();
                    }else{
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

                            NodeVector* neighs = this->overlay->neighborSet(maxServers);
                            EV << "nnnnnnnnnnnnnnnnnnnn" << thisNode.getIp() << " my overlay " << neighs->size() << " neighsbours\n";
                            for(std::vector<NodeHandle>::const_iterator it = neighs->begin(); it != neighs->end();it++){
                                EV << (*it).getKey().toString(16) << "\n";
                                allKeys.insert((*it).getKey());
                            }

                            inUse.insert(myKey);
                            clientAddTimer = new cMessage("Client: Add or Remove");
                            scheduleAt(simTime() + clientPeriod, clientAddTimer);

                            // start client timer
                            this->clientMoveTimer = new cMessage("ClientMove Timer");
                            scheduleAt(simTime() + 0.2, clientMoveTimer);

                            serverTimer = new cMessage("Server load check");
                            scheduleAt(simTime() + loadPeriod, serverTimer);

                            ticTimer = new cMessage("ticTimer");
                            scheduleAt(simTime()+1,ticTimer);

                            thisServer = new QuadServer(myKey, areaDim/2,areaDim/2);
                            thisServer->setMasterKey(myKey);
                            thisServer->addRect(Point(0,0), Point(areaDim,areaDim));
                            addClient();
                            area=thisServer->calcArea();

                            sCount=1;


                        }
                        delete msg;
                    }else{
                        // Unknown messages can be deleted
                        delete msg;
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
    case CLIENTSIZE_MSG : {
//        OverlayKey senderKey = myMsg->getSenderKey();
//        delete myMsg;
//        DLBMessage *myMessage; // the message we'll send
//        myMessage = new DLBMessage();
//        myMessage->setType(DEBUG_MSG); // set the message type to LOC_MSG
//        myMessage->setSenderKey(myKey);  // Store this
//        myMessage->setByteLength(100); // set the message length to 100 bytes
//
//
//        EV << "QuadtreeApp::deliver => " << thisNode.getIp() << ": Got packet from "
//           << senderKey << ", sending back!"
//           << std::endl;
//
//        callRoute(senderKey.toDouble(),myMsg);
    }break;
    case DEBUG_MSG : {
        set<OverlayKey> neighs = myMsg->getMyNeighs();
        for(set<OverlayKey>::iterator it= neighs.begin();it!=neighs.end();it++){
            allKeys.insert(*it);
        }
        EV << "nnnnnnnnnnnnnnnnnnnn" << thisNode.getIp() << "new neighs " << neighs.size() << " all neighsbours " << allKeys.size() << "\n";
        allKeysCount = allKeys.size();
        }delete msg; break;
    case SERVER_MSG : {
        myKey = this->overlay->getThisNode().getKey();

        // Create new QuadServer object and assign values to it
        thisServer = new QuadServer;
        *thisServer = myMsg->getQuadServer();

        set<OverlayKey> myKeys;
        NodeVector* neighs = this->overlay->neighborSet(maxServers);
        EV << "nnnnnnnnnnnnnnnnnnnn" << thisNode.getIp() << " my overlay " << neighs->size() << " neighsbours\n";
        for(std::vector<NodeHandle>::const_iterator it = neighs->begin(); it != neighs->end();it++){
            EV << (*it).getKey() << "\n";
            myKeys.insert((*it).getKey());
        }

        DLBMessage* newMsg = new DLBMessage();
        newMsg->setType(DEBUG_MSG);
        newMsg->setMyNeighs(myKeys);

        callRoute(thisServer->masterKey,newMsg);

        EV << "nnnnnnnnnnnnnnnnnnnnnn \n";


        this->clientCount = thisServer->myClients.size();
        EV << "+++++++++++++++++\nQuadtreeApp::deliver => " << myKey << " Got my new server state" << std::endl;
        EV << "Loc : (" << thisServer->loc.x() << "," << thisServer->loc.y() << ")\n"
             << "Key : " << thisServer->key << "\n"
             << "Neighbours size : " << thisServer->neighbours.size() << "\n"
             << "neighbours[0] : " << *thisServer->neighbours.begin() << "\n"
             << "myClients.size : " << thisServer->myClients.size() << "\n"
             << "Rec: tl(" << thisServer->cell.rect.front()->topLeft.x() << "," << thisServer->cell.rect.front()->topLeft.y() << ")"
             << "br(" << thisServer->cell.rect.front()->botRight.x() << "," << thisServer->cell.rect.front()->botRight.y() << ")"
             << "\n+++++++++++++++" << std::endl;

        updateNeighbours();
        area = thisServer->calcArea();

        // start client timer
        clientMoveTimer = new cMessage("ClientMove Timer");
        scheduleAt(simTime() + 0.5, clientMoveTimer);

        // start load Check timer
        serverTimer = new cMessage("Server load check");
        scheduleAt(simTime() + loadPeriod, serverTimer);

        ticTimer = new cMessage("ticTimer");
        scheduleAt(simTime()+1,ticTimer);

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
            overloadSet.insert(slaveKey);
            emit(overloadSig, overloadSet.size());
            delete myMsg;
            OverlayKey newKey = this->getNewServerKey(slaveKey);
            if (!newKey.isUnspecified()) {
                DLBMessage *myMessage; // the message we'll send
                myMessage = new DLBMessage();
                myMessage->setType(REQKEY_MSG); // set the message type
                myMessage->setSenderKey(newKey);  // Store the new serverkey in senderkey
                myMessage->setByteLength(sizeof(newKey)); // set the message length to size of an Overlaykey

                callRoute(slaveKey, myMessage);
                emit(controlSig, ++numControlMsg);
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
        QuadServer retServer = myMsg->getQuadServer();
        EV << "**************\nQuadtreeApp::deliver => " << myKey << " Got return server" << std::endl;
        EV << "Parent key : " << retServer.parent->key << std::endl;

        returnServer(&retServer);

        EV << "**************" << std::endl;

    }delete msg; break;
    case FREEME_MSG: {
        // Test to see if I am master
        if(master){
            inUse.erase(myMsg->getSenderKey());
            sCount = inUse.size();
            EV << "**************\n Erased inUse["<<myMsg->getSenderKey() << "]" << std::endl;
        }
    }delete msg; break;
    case NEIGH_REQ: {
        std::list<Rectangle*> rects = myMsg->getRects();
        OverlayKey senderKey = myMsg->getSenderKey();
        delete myMsg;
        if (thisServer->adjacent(&rects)) {
            thisServer->neighbours.insert(senderKey);
            DLBMessage* ackNeigh = new DLBMessage();
            ackNeigh->setType(NEIGH_A_R);
            ackNeigh->setSenderKey(myKey);
            callRoute(senderKey, ackNeigh);
            emit(controlSig, ++numControlMsg);
            EV << "QuadtreeApp::" << thisNode.getIp() <<" sending ACK to neighbour" << senderKey << std::endl;
        }
        this->neighCount = thisServer->neighbours.size();

    }break;
    case NEIGH_A_R : {
        OverlayKey removeKey = myMsg->getRemoveKey();

        if(!removeKey.isUnspecified()) {
            thisServer->neighbours.erase(removeKey);
            EV << "QuadtreeApp::" << thisNode.getIp() << " Remove key " << removeKey << std::endl;
        }

        thisServer->neighbours.insert(myMsg->getSenderKey());
        EV << "QuadtreeApp::" << thisNode.getIp() << " Inserting new neighbour " << myMsg->getSenderKey() << std::endl;
        this->neighCount = thisServer->neighbours.size();
    }delete myMsg; break;
    }
}

void QuadtreeApp::addClient() {
        if (globClientCount < maxClients){
            thisServer->myClients.insert(new Client(thisServer->loc, areaDim));
            clientCount++;
            globClientCount++;
            EV << "##############QuadtreeApp::addClient => " << thisNode.getIp() << " Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y();
            EV << "##############"<< std::endl;
        }
}

void QuadtreeApp::removeClient() {
    if (thisServer->myClients.size() > 0){
        EV << "##############QuadtreeApp::removeClient => " << thisNode.getIp() << " Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y();
        EV << "##############"<< std::endl;
//        delete *thisServer->myClients.rbegin();
        thisServer->myClients.erase(*thisServer->myClients.rbegin());
        clientCount--;
        globClientCount--;
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
        set <OverlayKey>::iterator sit;
        DLBMessage* clientTMsg = new DLBMessage();
        clientTMsg->setType(CLIENTRANS_MSG);
        clientTMsg->setSenderKey(myKey);
        clientTMsg->setClients(notMine);
        clientTMsg->setByteLength(sizeof(notMine)); // set the message length to the size of the vector notMine

        for(sit = thisServer->neighbours.begin(); sit != thisServer->neighbours.end(); sit++) {
            EV << "QuadTreeApp::clientUpdate => Transfer clients to " << *sit << std::endl;
            callRoute(*sit,clientTMsg->dup());
            emit(neighSig, ++numNeighMsg);
            numClientTrans += notMine.size();
            emit(clientTransSig, numClientTrans);
        }
        delete clientTMsg;
        EV << "------------ Transfer Clients--------------" << std::endl;
    }
}

void QuadtreeApp::checkLoad() {
    emit(clientOwn,thisServer->myClients.size());
    if (thisServer->isLoaded() && thisServer->lvl <2 ){
        if (master) {
            overloadSet.insert(myKey);
            emit(overloadSig, overloadSet.size());

            // Select new server key
            OverlayKey newKey =  getNewServerKey(myKey);
            if (newKey.isUnspecified()){
                newKey =  getNewServerKey(thisServer->key);
                if(newKey.isUnspecified())
                    return;
            }
            sendNewServer(newKey);
            overloadSet.erase(myKey);
        }else{
            EV << "@@@@@@@@@@@@@@@@@@ Slave overload @@@@@@@@@@@@@@@@" << std::endl;
            EV << "QuadTree::checkLoad => slave overloaded, requesting new server" << std::endl;
            DLBMessage* reqMsg = new DLBMessage();
            reqMsg->setType(REQKEY_MSG); // set the message type
            reqMsg->setSenderKey(myKey); // Set senderKey to myKey
            reqMsg->setByteLength(100); // set the message length to 100 bytes

            callRoute(thisServer->masterKey, reqMsg);
            emit(controlSig, ++numClientTrans);
            EV << "@@@@@@@@@@@@@@@@@@ Slave overload @@@@@@@@@@@@@@@@" << std::endl;
        }
    }else{
        if(thisServer->underLoaded()) {
            EV << "******************** Underload **********************" << std::endl;
            EV << "thisServer.parent.key " << thisServer->parent->key << "\n"
                    << "thisServer.lvl " << thisServer->lvl << "\n"
                    << "thisServer.parent.lvl " << thisServer->parent->lvl << "\n"
                    << "thisServer.cell.n " << thisServer->cell.n << std::endl;

            if (thisServer->parent != NULL && thisServer->parent->lvl == thisServer->lvl && thisServer->cell.n == 1) {

                EV << "QuadtreeApp::checkLoad " << thisNode.getIp() << " underload transfering area and clients to parent (if possible)" << std::endl;

//                endSimulation();
                DLBMessage* sretMsg = new DLBMessage();
                sretMsg->setType(RETSERV_MSG);
                sretMsg->setSenderKey(myKey);
                sretMsg->setQuadServer(*thisServer);
                sretMsg->setByteLength(sizeof(*thisServer));

                callRoute(thisServer->parent->key, sretMsg);
                emit(controlSig, ++numControlMsg);

                DLBMessage* freeMsg = new DLBMessage();
                freeMsg->setType(FREEME_MSG);
                freeMsg->setSenderKey(myKey);
                freeMsg->setByteLength(sizeof(myKey));

                callRoute(thisServer->masterKey, freeMsg);
                emit(controlSig, ++numControlMsg);


                numNeighMsg =0;
                numControlMsg =0;
                numClientTrans =0;
                emit(neighSig,numNeighMsg);
                emit(controlSig,numControlMsg);
                emit(clientTransSig, numClientTrans);

                clientCount = 0;
                area=0;
                //Cancel all timers
                cancelAndDelete(serverTimer);
                cancelAndDelete(clientMoveTimer);
//                cancelAndDelete(clientAddTimer);
//                cancelAndDelete(ticTimer);
                EV << "Cancel all timers" << std::endl;
                EV << "******************** Underload **********************" << std::endl;

            }
        }
    }
}

OverlayKey QuadtreeApp::getNewServerKey(OverlayKey key) {
//    std::vector<NodeHandle>::const_iterator it;
//    std::set<OverlayKey>::iterator kit;
////    NodeVector* neighs = this->overlay->neighborSet(maxServers);
//    NodeVector* neighs = this->overlay->local_lookup(key, maxServers, false);
//
//
//    if (sCount < maxServers) {
//        EV << "QuadtreeApp::checkLoad =>My " << thisNode.getIp() << " OverlayNeighbours size: " << neighs->size()-1 << std::endl;
//
//        for (it = neighs->begin(); it != neighs->end(); it++) {
//            kit = inUse.find((*it).getKey());
//            if (kit == inUse.end()) {
//                inUse.insert((*it).getKey());
//                EV << "VoronoiApp::checkLoad => My neighbour: " << (*it).getKey() << " Ip: " << (*it).getIp() << std::endl;
//                sCount = inUse.size();
//                return (*it).getKey();
//            }
//        }
//    }

    set<OverlayKey>::iterator it;
    std::set<OverlayKey>::iterator kit;
    if(sCount < maxServers){
        for(it=allKeys.begin(); it!=allKeys.end();it++){
            kit = inUse.find(*it);
            if (kit == inUse.end()){
                inUse.insert(*it);
                EV << "QuadtreeApp::checkLoad => My neighbour: " << (*it) << std::endl;
                sCount = inUse.size();
                return (*it);
            }
        }
    }

    return OverlayKey::UNSPECIFIED_KEY;
}

void QuadtreeApp::sendNewServer(OverlayKey newKey) {
    EV << "+++++++++++++++++ " << thisNode.getIp() << " SendNewServer ++++++++++++++++++" << std::endl;
    EV << "QuadtreeApp::sendNewServer to key: " << newKey << std::endl;
    QuadServer* newServer = new QuadServer(newKey); // Init a new server object with key=newKey
    newServer->setMasterKey(thisServer->masterKey); // Set master key used to request new server when loaded
    newServer->key = newKey;

    if (thisServer->transfer(newServer)) {
        clientCount = thisServer->myClients.size();
        DLBMessage *myMessage; // the message we'll send
        myMessage = new DLBMessage();
        myMessage->setType(SERVER_MSG); // set the message type to LOC_MSG
        myMessage->setSenderKey(myKey);  // Store this
        myMessage->setByteLength(sizeof(*newServer)); // set the message length to 100 bytes
        myMessage->setQuadServer(*newServer);

        EV << "QuadtreeApp::checkLoad => Overloaded and setting up new server, key: " << newKey << " MyLoc (" << thisServer->loc.x() << "," << thisServer->loc.y()
                << ") NewServloc (" << newServer->loc.x() << "," << newServer->loc.y() << ")"<< std::endl;
        callRoute(newKey, myMessage);
        emit(controlSig, ++numControlMsg);
    }
    area = thisServer->calcArea();
    EV << "+++++++++++++++++ SendNewServer ++++++++++++++++++" << std::endl;
}

void QuadtreeApp::returnServer(QuadServer* retServer) {

    set <Client*>::iterator cit;
    set <OverlayKey>::iterator it;

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
    DLBMessage *neighMsg = new DLBMessage();
    neighMsg->setType(NEIGH_A_R);
    neighMsg->setSenderKey(myKey);
    neighMsg->setRemoveKey(retServer->key);
    neighMsg->setByteLength(sizeof(myKey));

    // Remove me from all neighbour lists
    for(it = retServer->neighbours.begin(); it != retServer->neighbours.end(); it++) {
        callRoute(*it, neighMsg->dup());
        emit(neighSig, ++numNeighMsg);
    }
    delete neighMsg;
    retServer->lvl = -1;

    EV << "myClients.size() : " << retServer->myClients.size() << std::endl;
    EV << "myNeighbours.size() : " << retServer->neighbours.size() << std::endl;
    EV << "Lvl : " << retServer->lvl << std::endl;
}

void QuadtreeApp::updateNeighbours() {
    EV << "+++++++++++++++++ " << thisNode.getIp() << " UpdateNeighs ++++++++++++++++++" << std::endl;
    std::set<OverlayKey>::iterator it;

    DLBMessage *rectMsg = new DLBMessage();
    rectMsg->setType(NEIGH_REQ);
    rectMsg->setSenderKey(myKey);
    rectMsg->setRects(thisServer->cell.rect);
    rectMsg->setByteLength(sizeof(thisServer->cell.rect));

    for (it = thisServer->parent->neighbours.begin(); it != thisServer->parent->neighbours.end(); it++) {
        if(*it != myKey) {
            callRoute(*it, rectMsg->dup());
            emit(neighSig, ++numNeighMsg);
        }
    }

    delete rectMsg;
    EV << "+++++++++++++++++ UpdateNeighs ++++++++++++++++++" << std::endl;
}
