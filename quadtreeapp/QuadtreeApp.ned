//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
// @author Antonio Zea
//

import oversim.common.BaseApp;
import oversim.common.ITier;

package oversim.DLB.quadtreeapp;

simple QuadtreeApp extends BaseApp
{
    parameters:
        @class(QuadtreeApp);
        int maxServers;            // largest key we can pick
        int maxClients;
        int areaDim;
        double clientPeriod;
        double loadPeriod;
        double leaveChance;      // chance that a client leaves
        
        // number of Server signals
        @signal[numOverloadServ](type="int");
        @statistic[numOverloadServ](title="number of overload servers"; source="numOverloadServ"; record=vector,stats; interpolationmode=none);
        @signal[numInUseServ](type="int");
        @statistic[numInUseServ](title="number of in use servers"; source="numInUseServ"; record=vector,stats; interpolationmode=none);
        @signal[numFreeServ](type="int");
        @statistic[numFreeServ](title="number of free servers"; source="numFreeServ"; record=vector,stats; interpolationmode=none);
        
        @signal[numNeighMsg](type="int");
        @statistic[numNeighMsg](title="number of neighbour msg"; source="numNeighMsg"; record=vector,stats; interpolationmode=none);
     	@signal[numClients](type="int");	
        @statistic[numClients](title="number of Client migrations"; source="numClients"; record=vector,stats; interpolationmode=none);
        @signal[numControlMsg](type="int");	
        @statistic[numControlMsg](title="number of control msg"; source="numControlMsg"; record=vector,stats; interpolationmode=none);
        
        @signal[numClientOwn](type="int");
        @statistic[numClientOwn](title="number of Client owned"; source="numClientOwn"; record=vector,stats; interpolationmode=none);
        @signal[clientDens](type="double");
        @statistic[clientDens](title="Client density"; source="clientDens"; record=vector,stats; interpolationmode=none);
}

module QuadtreeAppModules like ITier
{
    gates:
        input udpIn;             // gate from the UDP layer
        output udpOut;           // gate to the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        output to_lowerTier;     // gate to the lower tier
        output to_upperTier;     // gate to the upper tier
        input trace_in;          // gate for trace file commands
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer

    submodules:
        application: QuadtreeApp;
        
    connections allowunconnected:
        from_lowerTier --> application.from_lowerTier;
        to_lowerTier <-- application.to_lowerTier;
        udpIn --> application.udpIn;
        udpOut <-- application.udpOut;
        tcpIn --> application.tcpIn;
        tcpOut <-- application.tcpOut;
        trace_in --> application.trace_in;
        
}
