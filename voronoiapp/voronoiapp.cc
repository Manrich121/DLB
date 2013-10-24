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
#include "voronoiapp.h"
#include "NeighborCache.h"

#include "DLBMessage_m.h"


#define MOVTIME 0.5

// This line tells the simulator that VoronoiApp is going to be extended using C++ code.
// It *must* be present (only once) somewhere in your code, outside a function, for each module you'll be extending.
Define_Module(VoronoiApp);

// initializeApp() is called when the module is being created.
// Use this function instead of the constructor for initializing variables.
void VoronoiApp::initializeApp(int stage)
{
    // initializeApp will be called twice, each with a different stage.
    // stage can be either MIN_STAGE_APP (this module is being created),
    // or MAX_STAGE_APP (all modules were created).
    // We only care about MIN_STAGE_APP here.

    if (stage != MIN_STAGE_APP) return;

//    srand(time(NULL));
    // Get params set in .ini file
    maxServers = par("maxServers");
    areaDim = par("areaDim");
    maxClients = par("maxClients");
    clientPeriod = par("clientPeriod");
    loadPeriod = par("loadPeriod");
    leaveChance = par("leaveChance");
    clientCount = 0;
    globClientCount = 0;
    neighCount = 0;
    cellN = 0;
    area = 0;
    myKey = OverlayKey::ZERO;
    thisServer = NULL;

    numNeighMsg =0;
    numOverloadMsg =0;
    numNewServerMsg =0;
    numFreeMsg =0;
    numClientTrans =0;
    clientDens =0;

    //TODO: add WATCH on data vars to record
    WATCH(sCount);
    WATCH(globClientCount);
    WATCH(clientCount);
    WATCH(neighCount);
    WATCH(cellN);
    WATCH(area);
    WATCH(clientDens);
    WATCH(myKey);

    // Register signals
    overloadSig = registerSignal("numOverloadServ");
    inUseSig = registerSignal("numInUseServ");
    freeServSig = registerSignal("numFreeServ");
    neighSig = registerSignal("numNeighMsg");
    clientTransSig = registerSignal("numClients");
    clientOwn = registerSignal("numClientOwn");
    totalClients = registerSignal("totalClients");
    clientD = registerSignal("clientDens");

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
void VoronoiApp::finishApp(){
}

// handleTimerEvent() is called when a self message is received
void VoronoiApp::handleTimerEvent(cMessage* msg){
    // is this Tic timer?
    if (msg == ticTimer) {
        scheduleAt(simTime() + 1, ticTimer);
        if(master){
            emit(inUseSig, inUse.size());
            emit(overloadSig, overloadSet.size());
            emit(freeServSig, maxServers-inUse.size());
            int sum =thisServer->myClients.size();
            // Calculate total clients
            for(std::map<OverlayKey,int>::iterator it = clientCountMap.begin();it != clientCountMap.end();it++){
                sum += (*it).second;
            }
            emit(totalClients,sum);
        }else{

            DLBMessage* clientsMsg = new DLBMessage();
            clientsMsg->setType(CLIENTSIZE_MSG);
            clientsMsg->setSenderKey(myKey);
            clientsMsg->setClientSize(thisServer->myClients.size());

            callRoute(thisServer->masterKey, clientsMsg);
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
                    scheduleAt(simTime() + MOVTIME, clientMoveTimer);
                    if (thisServer != NULL) {
                        if (thisServer->myClients.size() > 0) {
                            clientUpdate();
                        }
                    }
                }else{
                    if (msg == setupMessage){
                        myKey = this->overlay->getThisNode().getKey();
                        sCount = 0;
                        if(master){
                            inUse.insert(myKey);
                            clientAddTimer = new cMessage("Client: Add or Remove");
                            scheduleAt(simTime() + clientPeriod, clientAddTimer);

                            // start client timer
                            this->clientMoveTimer = new cMessage("ClientMove Timer");
                            scheduleAt(simTime() + MOVTIME, clientMoveTimer);

                            serverTimer = new cMessage("Server load check");
                            scheduleAt(simTime() + loadPeriod, serverTimer);

                            ticTimer = new cMessage("ticTimer");
                            scheduleAt(simTime()+1,ticTimer);

                            thisServer = new VoroServer(myKey, areaDim/2,areaDim/2, areaDim);
                            // Add first area
                            std::vector<Point> p;
                            p.push_back(Point(areaDim,0));
                            p.push_back(Point(areaDim,areaDim));
                            p.push_back(Point(0,0));
                            p.push_back(Point(0,areaDim));
                            thisServer->GrahamScan(p);
                            area=thisServer->calcArea();

                            thisServer->setMasterKey(myKey);
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
void VoronoiApp::deliver(OverlayKey& key, cMessage* msg) {
    // we are only expecting messages of type DLBMessage, throw away any other
    DLBMessage *myMsg = dynamic_cast<DLBMessage*>(msg);
    if (myMsg == NULL) {
        delete msg; // type unknown!
        return;
    }

    switch (myMsg->getType()){
    case CLIENTSIZE_MSG : {
        if(master){
           this->clientCountMap[myMsg->getSenderKey()] = myMsg->getClientSize();
        }

    }delete myMsg; break;
    case DEBUG_MSG : {
            EV << "VoronoiApp::deliver => " << thisNode.getIp() << ": Got reply from "<< myMsg->getSenderKey() << std::endl;
        }delete msg; break;
    case SERVER_MSG : {
        myKey = this->overlay->getThisNode().getKey();
        // Create new VoroServer object and assign values to it
        thisServer = new VoroServer(myKey, areaDim);
        *thisServer = myMsg->getVoroServer();
        this->clientCount = thisServer->myClients.size();
        EV << "+++++++++++++++++\nVoronoiApp::deliver => " << myKey << " Got my new server state" << std::endl;
        EV << "Loc : (" << thisServer->loc.x() << "," << thisServer->loc.y() << ")\n"
             << "Key : " << thisServer->key << "\n"
             << "Neighbours size : " << thisServer->neighbours.size() << "\n"
             << "neighbours[0] : " << (*thisServer->neighbours.begin()).first << "\n"
             << "myClients.size : " << thisServer->myClients.size() << "\n"
             << "cell.n : " << thisServer->cell.n << std::endl;
        cellN = thisServer->cell.n;
        area = thisServer->calcArea();
        EV << "+++++++++++++++" << std::endl;

        // start client timer
        clientMoveTimer = new cMessage("ClientMove Timer");
        scheduleAt(simTime() + MOVTIME, clientMoveTimer);

//      start load Check timer
        serverTimer = new cMessage("Server load check");
        scheduleAt(simTime() + loadPeriod, serverTimer);

        ticTimer = new cMessage("ticTimer");
        scheduleAt(simTime()+1,ticTimer);

        this->neighCount = thisServer->neighbours.size();
        }delete msg; break;
    case CLIENTRANS_MSG: {
        EV << "------------------ Client Transfer ----------------" << std::endl;
//                  Check ownership on new clients and insert into thisServer.myClients
        std::vector<Client*> maybeMine = myMsg->getClients();
        std::vector<Client*> mine;
        for(unsigned int i=0;i<maybeMine.size();i++){
            EV << "VoronoiApp::deliver => Client (" << maybeMine.at(i)->loc.x() << "," << maybeMine.at(i)->loc.y() << ") Mine? " <<  thisServer->ownership(maybeMine.at(i))  << std::endl;
            if(thisServer->ownership(maybeMine.at(i))){
                thisServer->myClients.insert(maybeMine.at(i));
                mine.push_back(maybeMine.at(i));
            }
        }
        this->clientCount = thisServer->myClients.size();
        if(!mine.empty()){
            OverlayKey senderKey = myMsg->getSenderKey();
            delete myMsg;
            DLBMessage *mineMsg = new DLBMessage();
            mineMsg->setType(CLIENTT_ACK);
            mineMsg->setSenderKey(myKey);
            mineMsg->setClients(mine);

            callRoute(senderKey, mineMsg);
            EV << "Sending CLIENT T_ACK to " << senderKey << " with " << mineMsg->getClients().size() << " client acks" << std::endl;
        }else{
            delete myMsg;
        }

        EV << "------------------ Client Transfer ----------------" << std::endl;
        }break;
    case CLIENTT_ACK: {
            std::vector<Client*> remove = myMsg->getClients();
            for(unsigned int i=0;i<remove.size();i++){
                thisServer->myClients.erase(remove[i]);
                EV << "^^^^clientTransferACK: Remove client: " << remove[i]->loc.x() << ", " << remove[i]->loc.y() << std::endl;
            }
            this->clientCount = thisServer->myClients.size();
    }delete myMsg; break;
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
                overloadSet.erase(slaveKey);
//  TODO:       emit(msgCountSig, ++msgCount);
            }
        }else{
            EV << "@@@@@@@@@@@@@@@@@@" << std::endl;
            OverlayKey newKey = myMsg->getSenderKey();
            sendNewServer(newKey);
            delete msg;
        }
        }break;
    case RETSERV_MSG: {
        // Create new VoroServer object and assign values to it
        VoroServer retServer = myMsg->getVoroServer();
        EV << "**************\nVoronoiApp::deliver => " << myKey << " Got return server" << std::endl;
        EV << "Return server key : " << retServer.key << std::endl;

        returnServer(&retServer);
        EV << "**************" << std::endl;

    }delete msg; break;
    case FREEME_MSG: {
        // Test to see if I am master
        if(master){
            inUse.erase(myMsg->getSenderKey());
            clientCountMap.erase(myMsg->getSenderKey());
            sCount = inUse.size();
            EV << "**************\n Erased inUse["<<myMsg->getSenderKey() << "]" << std::endl;
        }
    }delete msg; break;
    case NEIGH_REQ: {
        OverlayKey senderKey = myMsg->getSenderKey();

        if (thisServer->isNeigh(myMsg->getSenderLoc())){
            thisServer->neighbours[senderKey] = myMsg->getSenderLoc();

            thisServer->generateVoronoi();          // Recalculate voronoi diagram
            cellN = thisServer->cell.n;
            area = thisServer->calcArea();

//            DLBMessage* ackNeigh = new DLBMessage();
//            ackNeigh->setType(NEIGH_A_R);
//            ackNeigh->setSenderKey(myKey);
//            ackNeigh->setSenderLoc(thisServer->loc);
//            ackNeigh->setRemoveKey(OverlayKey::UNSPECIFIED_KEY);
//
//            callRoute(senderKey, ackNeigh);
//            emit(msgCountSig, 1);
            EV << "VoronoiApp::" << thisNode.getIp() <<" Inserting my new neighbour " << senderKey << std::endl;
        }
        this->neighCount = thisServer->neighbours.size();
    }delete myMsg;break;
    case NEIGH_A_R : {
        OverlayKey removeKey = myMsg->getRemoveKey();
        EV << "NEIGH_ADD_REMOVE:: " << thisNode.getIp() << "\n";
        EV << "RemoveKey.unspec " << removeKey.isUnspecified() << "\n";
        EV << "AddKey " << myMsg->getSenderKey() << "\n";
        EV << "isNeigh " << thisServer->isNeigh(myMsg->getSenderLoc()) << "\n";

        if(!removeKey.isUnspecified()) {
            thisServer->neighbours.erase(removeKey);
            EV << "VoronoiApp::" << thisNode.getIp() << " Remove key " << removeKey << std::endl;
        }

        OverlayKey senderKey = myMsg->getSenderKey();
        if (!senderKey.isUnspecified()){
            if (thisServer->isNeigh(myMsg->getSenderLoc())){
                thisServer->neighbours[senderKey] = myMsg->getSenderLoc();
                EV << "VoronoiApp::" << thisNode.getIp() << " Inserting new neighbour " << senderKey << std::endl;
            }
        }
        EV << "VoronoiApp::" << thisNode.getIp() << " Neighbours size : " << thisServer->neighbours.size() << "\n";
        thisServer->generateVoronoi();          // Recalculate voronoi diagram
        cellN = thisServer->cell.n;
        this->neighCount = thisServer->neighbours.size();

        std::vector<Client*> maybeMine = myMsg->getClients();
        if (!maybeMine.empty()) {
            for(unsigned int i=0;i<maybeMine.size();i++){
                EV << "VoronoiApp::deliver => Client (" << maybeMine.at(i)->loc.x() << "," << maybeMine.at(i)->loc.y() << ") Mine? " <<  thisServer->ownership(maybeMine.at(i))  << std::endl;
                if(thisServer->ownership(maybeMine.at(i))){
                    thisServer->myClients.insert(maybeMine.at(i));
                }
            }
            this->clientCount = thisServer->myClients.size();
        }
    }delete myMsg; break;
    }
}

void VoronoiApp::addClient() {
    if (globClientCount < maxClients){
        thisServer->myClients.insert(new Client(thisServer->loc, areaDim));
        clientCount++;
        globClientCount++;
        EV << "##############VoronoiApp::addClient => " << thisNode.getIp() << " Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y();
        EV << "##############"<< std::endl;
    }
}

void VoronoiApp::removeClient() {
    if (thisServer->myClients.size() > 0){
        EV << "##############VoronoiApp::removeClient => " << thisNode.getIp() << " Client: " << (*thisServer->myClients.rbegin())->loc.x() << ", " << (*thisServer->myClients.rbegin())->loc.y();
        EV << "##############"<< std::endl;
//        delete *thisServer->myClients.rbegin();
        thisServer->myClients.erase(*thisServer->myClients.rbegin());
        clientCount--;
        globClientCount--;
    }
}

void VoronoiApp::clientUpdate() {
    set <Client*>::iterator it;
    set <Client*>::iterator tmp;
    std::vector<Client*> notMine;


    EV << "------------ "<< thisNode.getIp()<< " Update " << thisServer->myClients.size() << " Clients--------------" << std::endl;
    for (it = thisServer->myClients.begin(); it != thisServer->myClients.end();it++) {
       (*it)->move();
//       EV << "VoronoiApp::clientUpdate: Update client movement: " << (*it)->loc.x() << ", " << (*it)->loc.y() << std::endl;
       if (!thisServer->ownership(*it)){
           EV << "VoronoiApp::clientUpdate: Transfer client: " << (*it)->loc.x() << ", " << (*it)->loc.y() << std::endl;
           notMine.push_back(*it);
//           thisServer->myClients.erase(*it);
//           clientCount--;
       }
    }

    if (notMine.size() > 0) {
        EV << "------------Transfer Clients--------------" << std::endl;
        EV << "notMine.size() : " << notMine.size() << "\n"
           << "neighbours.size() : " << thisServer->neighbours.size() << std::endl;

        // Transfer clients to all my neighbours
        map <OverlayKey, Point>::iterator sit;
        DLBMessage* clientTMsg = new DLBMessage();
        clientTMsg->setType(CLIENTRANS_MSG);
        clientTMsg->setSenderKey(myKey);
        clientTMsg->setClients(notMine);
        clientTMsg->setByteLength(sizeof(notMine)); // set the message length to the size of the vector notMine

        for(sit = thisServer->neighbours.begin(); sit != thisServer->neighbours.end(); sit++) {
            EV << "VoronoiApp::clientUpdate => Transfer clients to " << (*sit).first << std::endl;
            callRoute((*sit).first,clientTMsg->dup());
//TODO:            emit(msgCountSig, ++msgCount);
            emit(clientTransSig, notMine.size());
        }
        delete clientTMsg;
        EV << "------------ Transfer Clients--------------" << std::endl;
    }
}

void VoronoiApp::checkLoad() {
    emit(clientOwn,thisServer->myClients.size());
    if (thisServer->isLoaded()){
        if (master) {
            overloadSet.insert(myKey);
            emit(overloadSig, overloadSet.size());

            // Select new server key
            OverlayKey newKey =  getNewServerKey(myKey);
            if (newKey.isUnspecified()){
                newKey =  getNewServerKey(thisServer->masterKey);
                if(newKey.isUnspecified())
                   return;
            }
            sendNewServer(newKey);
            overloadSet.erase(myKey);
        }else{
            EV << "@@@@@@@@@@@@@@@@@@ Slave overload @@@@@@@@@@@@@@@@" << std::endl;
            EV << "VoronoiApp::checkLoad => slave overloaded, requesting new server" << std::endl;
            DLBMessage* reqMsg = new DLBMessage();
            reqMsg->setType(REQKEY_MSG); // set the message type
            reqMsg->setSenderKey(myKey); // Set senderKey to myKey
            reqMsg->setByteLength(100); // set the message length to 100 bytes

            callRoute(thisServer->masterKey, reqMsg);
//TODO:            emit(msgCountSig, ++msgCount);
            EV << "@@@@@@@@@@@@@@@@@@ Slave overload @@@@@@@@@@@@@@@@" << std::endl;
        }
    }else{
        if(thisServer->underLoaded()) {
            if (this->master == false) {

                EV << "******************** Underload **********************" << std::endl;
                EV << "VoronoiApp::checkLoad " << thisNode.getIp() << " underload transfering area and clients neighbours (if possible)" << std::endl;

//                endSimulation();
                map <OverlayKey, Point>::iterator sit;
                DLBMessage* sretMsg = new DLBMessage();
                sretMsg->setType(RETSERV_MSG);
                sretMsg->setSenderKey(myKey);
                sretMsg->setVoroServer(*thisServer);
                sretMsg->setByteLength(sizeof(*thisServer));

                for(sit = thisServer->neighbours.begin(); sit != thisServer->neighbours.end(); sit++) {
                    EV << "VoronoiApp::underload => Notify neihgbour about my leave " << (*sit).first << std::endl;
                    callRoute((*sit).first,sretMsg->dup());
                    emit(neighSig, ++numNeighMsg);
                }

                delete sretMsg;

                DLBMessage* freeMsg = new DLBMessage();
                freeMsg->setType(FREEME_MSG);
                freeMsg->setSenderKey(myKey);
                freeMsg->setByteLength(sizeof(myKey));

                callRoute(thisServer->masterKey, freeMsg);
//TODO:                emit(msgCountSig, ++msgCount);

                clientCount = 0;
                neighCount = 0;
                thisServer->myClients.clear();
                thisServer->neighbours.clear();
                thisServer->deleteCell();
                //Cancel all timers
                cancelAndDelete(serverTimer);
                cancelAndDelete(clientMoveTimer);
                cancelAndDelete(ticTimer);
//                cancelAndDelete(clientAddTimer);
//                cancelAndDelete(ticTimer);
                EV << "Cancel all timers" << std::endl;
                EV << "******************** Underload **********************" << std::endl;

            }
        }
    }
    this->neighCount = thisServer->neighbours.size();
}

OverlayKey VoronoiApp::getNewServerKey(OverlayKey key) {
    std::vector<NodeHandle>::const_iterator it;
    std::set<OverlayKey>::iterator kit;
//    NodeVector* neighs = this->overlay->neighborSet(maxServers);
    NodeVector* neighs = this->overlay->local_lookup(key, maxServers, false);

    if (sCount < maxServers) {
        EV << "VoronoiApp::checkLoad =>My " << thisNode.getIp() << " OverlayNeighbours size: " << neighs->size()-1 << std::endl;

        for (it = neighs->begin(); it != neighs->end(); it++) {
            kit = inUse.find((*it).getKey());
            if (kit == inUse.end()) {
                inUse.insert((*it).getKey());
                EV << "VoronoiApp::checkLoad => My neighbour: " << (*it).getKey() << " Ip: " << (*it).getIp() << std::endl;
                sCount = inUse.size();
                return (*it).getKey();
            }
        }
    }
    return OverlayKey::UNSPECIFIED_KEY;
}

void VoronoiApp::sendNewServer(OverlayKey newKey) {
    EV << "+++++++++++++++++ " << thisNode.getIp() << " SendNewServer ++++++++++++++++++" << std::endl;
    EV << "VoronoiApp::sendNewServer to key: " << newKey << std::endl;
    VoroServer* newServer = new VoroServer(newKey, areaDim); // Init a new server object with key=newKey
    newServer->setMasterKey(thisServer->masterKey); // Set master key used to request new server when loaded
    newServer->key = newKey;

    EV << "newServer.areaDim" << newServer->areaDim << std::endl;
    thisServer->refine(newServer);
    clientCount = thisServer->myClients.size();
    cellN = thisServer->cell.n;
    area = thisServer->calcArea();
    updateNeighbours(newServer);

    DLBMessage *myMessage; // the message we'll send
    myMessage = new DLBMessage();
    myMessage->setType(SERVER_MSG); // set the message type to LOC_MSG
    myMessage->setSenderKey(myKey);  // Store this
    myMessage->setByteLength(sizeof(*newServer)); // set the message length to 100 bytes
    myMessage->setVoroServer(*newServer);

    EV << "VoronoiApp::checkLoad => Overloaded and setting up new server, key: " << newKey << " MyLoc (" << thisServer->loc.x() << "," << thisServer->loc.y()
            << ") NewServloc (" << newServer->loc.x() << "," << newServer->loc.y() << ")"<< std::endl;
    callRoute(newKey, myMessage);
//TODO:    emit(msgCountSig, ++msgCount);

    EV << "+++++++++++++++++ SendNewServer ++++++++++++++++++" << std::endl;
}

void VoronoiApp::returnServer(VoroServer* retServer) {

    set <Client*>::iterator cit;
    map <OverlayKey, Point>::iterator it;

    std::vector<Client*> output(retServer->myClients.begin(), retServer->myClients.end());

    EV << "Update neigbours" << std::endl;
    DLBMessage *removeMsg = new DLBMessage();
    removeMsg->setType(NEIGH_A_R);
    removeMsg->setSenderKey(OverlayKey::UNSPECIFIED_KEY);
    removeMsg->setRemoveKey(retServer->key);
    removeMsg->setClients(output);
    removeMsg->setByteLength(sizeof(myKey));

    // Send to removeMsg myself
//    callRoute(myKey, removeMsg->dup());
//    emit(neighSig, ++numNeighMsg);

    thisServer->neighbours.erase(retServer->key);
    thisServer->generateVoronoi();
    cellN = thisServer->cell.n;
    this->neighCount = thisServer->neighbours.size();

    map <OverlayKey, Point> excludeNeighs = retServer->neighbours;

    // Remove retServer from all my neighbour's lists (not already messaged)
    for(it = thisServer->neighbours.begin(); it != thisServer->neighbours.end(); it++) {
        if(excludeNeighs.find((*it).first) == excludeNeighs.end()){     // Not in excludeList
            callRoute((*it).first, removeMsg->dup());
            emit(neighSig, ++numNeighMsg);
        }
    }

    // Test retServer's neighbours neighbour?
//    for (it = excludeNeighs.begin(); it != excludeNeighs.end();it++){
//        if ((*it).first != myKey && thisServer->isNeigh((*it).second)){
//            thisServer->neighbours[(*it).first] = (*it).second;
//            EV << "VoronoiApp::" << thisNode.getIp() << " Inserting new neighbour " << (*it).first << std::endl;
//        }
//    }
    delete removeMsg;
    EV << "myClients.size() : " << retServer->myClients.size() << std::endl;
    EV << "myNeighbours.size() : " << retServer->neighbours.size() << std::endl;
    thisServer->generateVoronoi();

    EV << "Transfer all myClients" << std::endl;
    // Transfer all myClients
    for (cit = retServer->myClients.begin(); cit != retServer->myClients.end();cit++) {
        if(thisServer->ownership(*cit))
            thisServer->myClients.insert(*cit);
    }
    this->clientCount = thisServer->myClients.size();

}

void VoronoiApp::updateNeighbours(VoroServer* newServer) {
    EV << "+++++++++++++++++ " << thisNode.getIp() << " UpdateNeighs ++++++++++++++++++" << std::endl;
    std::map<OverlayKey, Point>::iterator it;

    DLBMessage *rectMsg = new DLBMessage();
    rectMsg->setType(NEIGH_REQ);
    rectMsg->setSenderKey(newServer->key);
    rectMsg->setSenderLoc(newServer->loc);
    rectMsg->setByteLength(sizeof(thisServer->loc));

    for (it = thisServer->neighbours.begin(); it != thisServer->neighbours.end(); it++) {
        if((*it).first != newServer->key) {
            callRoute((*it).first, rectMsg->dup());
            emit(neighSig, ++numNeighMsg);
        }
    }

//    thisServer->neighbours.clear();
    delete rectMsg;
    EV << "+++++++++++++++++ UpdateNeighs ++++++++++++++++++" << std::endl;
}


