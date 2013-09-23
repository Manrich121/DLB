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

    //TODO: add WATCH on data vars to record

    if (this->getParentModule()->getParentModule()->getIndex() == 0) {
        this->master = true;
        // start our timer!
        ticTimer = new cMessage("QuadTreeApp Timer");
        scheduleAt(simTime() + 1, ticTimer);
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
        }

        // Timer used to chech load
        if (msg == serverTimer) {

        }

        // Timer that adds or removes clients
        if (msg == clientAddTimer) {

        }

        // Timer used to recalculate all client locations
        if (msg == clientMoveTimer) {

        }

//      TODO: delete msg if not any of timer messages
}

// deliver() is called when a message is received from the overlay
// Unknown packets can be safely deleted here.
void QuadtreeApp::deliver(OverlayKey& key, cMessage* msg) {

}

