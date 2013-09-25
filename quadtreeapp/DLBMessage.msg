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
}}
class noncobject OverlayKey;
class noncobject QuadServer;

cplusplus {{
#include <vector>
#include "Client.h"
typedef std::vector<Client*> clientVect;
}}
class noncobject clientVect;

enum MessageType 
{
    LOC_MSG = 1;            // message used to send a location
    SERVER_MSG = 2;           // message used to check a server load
    CLIENTRANS_MSG = 3;		// message used to transfer a client
    DEBUG_MSG = 4;			// message used to test ping-pong funtions
}

packet DLBMessage {

    int type enum(MessageType);      // message type
	QuadServer transferServer;
	OverlayKey senderKey; 			// The key of the sender of this message
    clientVect clients;
}
