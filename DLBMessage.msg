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

cplusplus {{
#include <OverlayKey.h>
#include "QuadServer.h"
#include "VoroServer.h"
}}
class noncobject OverlayKey;
class noncobject QuadServer;
class noncobject VoroServer;

cplusplus {{
#include <vector>
#include "Client.h"
#include "Point.h"
typedef std::vector<Client*> clientVect;
typedef std::list<Rectangle*> rectVect;
}}
class noncobject clientVect;
class noncobject rectVect;
class noncobject Point;

enum MessageType 
{
    CLIENTSIZE_MSG = 1;            // message used to send a location
    SERVER_MSG = 2;         // message used to transfer a server
    CLIENTRANS_MSG = 3;		// message used to transfer a client
    REQKEY_MSG = 4;			// message used by slave to request a new serverkey (Stored in senderKey)
    RETSERV_MSG = 5;		// message used to return a child server to its parent
    FREEME_MSG = 6;			// message used to free a child server after returning to its parent
    NEIGH_REQ = 7;			// message used to transfer thisServer's rectangles
    NEIGH_A_R = 8;			// message used to add or remove the sender as a neighbour
    DEBUG_MSG = 9;			// message used to test ping-pong funtions
    CLIENTT_ACK = 10;
}

packet DLBMessage {
	// General
    int type enum(MessageType);      // message type
	OverlayKey senderKey; 			// The key of the sender of this message
    clientVect clients;				// A vector of client pointers to be check for ownerships
 	OverlayKey removeKey;			// indicating if requesting neighbour add or remove 	
    // Quadtree
	QuadServer quadServer;			// Used to transfer a quadtree server
	rectVect rects;					// a list of rectangles send to be evaluated for neighbourship
	// Voronoi
	VoroServer voroServer;			// Used to transfer a voronoi server
	Point senderLoc;
	int clientSize;					// Used to notify about client size
    
    
   
}
