//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 


import oversim.common.BaseApp;
import oversim.common.ITier;

package oversim.DLB.voronoiapp;

simple VoronoiApp extends BaseApp
{
    parameters:
        @class(VoronoiApp);
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
        @signal[numClientOwn](type="int");
        @statistic[numClientOwn](title="number of Client owned"; source="numClientOwn"; record=vector,stats; interpolationmode=none);
        @signal[totalClients](type="int");
        @statistic[totalClients](title="total number of clients"; source="totalClients"; record=vector,stats; interpolationmode=none);
        @signal[clientDens](type="double");
        @statistic[clientDens](title="Client density"; source="clientDens"; record=vector,stats; interpolationmode=none);
         
}

module VoronoiAppModules like ITier
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
        application: VoronoiApp;
        
    connections allowunconnected:
        from_lowerTier --> application.from_lowerTier;
        to_lowerTier <-- application.to_lowerTier;
        udpIn --> application.udpIn;
        udpOut <-- application.udpOut;
        tcpIn --> application.tcpIn;
        tcpOut <-- application.tcpOut;
        trace_in --> application.trace_in;
        
}
