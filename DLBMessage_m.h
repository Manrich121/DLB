//
// Generated file, do not edit! Created by opp_msgc 4.2 from DLB/DLBMessage.msg.
//

#ifndef _DLBMESSAGE_M_H_
#define _DLBMESSAGE_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0402
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif

// cplusplus {{
#include <OverlayKey.h>
#include "QuadServer.h"
#include "VoroServer.h"
// }}

// cplusplus {{
#include <vector>
#include "Client.h"
#include "Point.h"
typedef std::vector<Client*> clientVect;
typedef std::list<Rectangle*> rectVect;
// }}



/**
 * Enum generated from <tt>DLB/DLBMessage.msg</tt> by opp_msgc.
 * <pre>
 * enum MessageType 
 * {
 *     LOC_MSG = 1;            
 *     SERVER_MSG = 2;         
 *     CLIENTRANS_MSG = 3;		
 *     REQKEY_MSG = 4;			
 *     RETSERV_MSG = 5;		
 *     FREEME_MSG = 6;			
 *     NEIGH_REQ = 7;			
 *     NEIGH_A_R = 8;			
 *     DEBUG_MSG = 9;			
 *     CLIENTT_ACK = 10;
 * }
 * </pre>
 */
enum MessageType {
    LOC_MSG = 1,
    SERVER_MSG = 2,
    CLIENTRANS_MSG = 3,
    REQKEY_MSG = 4,
    RETSERV_MSG = 5,
    FREEME_MSG = 6,
    NEIGH_REQ = 7,
    NEIGH_A_R = 8,
    DEBUG_MSG = 9,
    CLIENTT_ACK = 10
};

/**
 * Class generated from <tt>DLB/DLBMessage.msg</tt> by opp_msgc.
 * <pre>
 * packet DLBMessage {
 * 	
 *     int type enum(MessageType);      
 * 	OverlayKey senderKey; 			
 *     clientVect clients;				
 *  	OverlayKey removeKey;			
 *     
 * 	QuadServer quadServer;			
 * 	rectVect rects;					
 * 	
 * 	VoroServer voroServer;			
 * 	Point senderLoc;
 * 
 *     
 *     
 *    
 * }
 * </pre>
 */
class DLBMessage : public ::cPacket
{
  protected:
    int type_var;
    OverlayKey senderKey_var;
    clientVect clients_var;
    OverlayKey removeKey_var;
    QuadServer quadServer_var;
    rectVect rects_var;
    VoroServer voroServer_var;
    Point senderLoc_var;

  private:
    void copy(const DLBMessage& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const DLBMessage&);

  public:
    DLBMessage(const char *name=NULL, int kind=0);
    DLBMessage(const DLBMessage& other);
    virtual ~DLBMessage();
    DLBMessage& operator=(const DLBMessage& other);
    virtual DLBMessage *dup() const {return new DLBMessage(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getType() const;
    virtual void setType(int type);
    virtual OverlayKey& getSenderKey();
    virtual const OverlayKey& getSenderKey() const {return const_cast<DLBMessage*>(this)->getSenderKey();}
    virtual void setSenderKey(const OverlayKey& senderKey);
    virtual clientVect& getClients();
    virtual const clientVect& getClients() const {return const_cast<DLBMessage*>(this)->getClients();}
    virtual void setClients(const clientVect& clients);
    virtual OverlayKey& getRemoveKey();
    virtual const OverlayKey& getRemoveKey() const {return const_cast<DLBMessage*>(this)->getRemoveKey();}
    virtual void setRemoveKey(const OverlayKey& removeKey);
    virtual QuadServer& getQuadServer();
    virtual const QuadServer& getQuadServer() const {return const_cast<DLBMessage*>(this)->getQuadServer();}
    virtual void setQuadServer(const QuadServer& quadServer);
    virtual rectVect& getRects();
    virtual const rectVect& getRects() const {return const_cast<DLBMessage*>(this)->getRects();}
    virtual void setRects(const rectVect& rects);
    virtual VoroServer& getVoroServer();
    virtual const VoroServer& getVoroServer() const {return const_cast<DLBMessage*>(this)->getVoroServer();}
    virtual void setVoroServer(const VoroServer& voroServer);
    virtual Point& getSenderLoc();
    virtual const Point& getSenderLoc() const {return const_cast<DLBMessage*>(this)->getSenderLoc();}
    virtual void setSenderLoc(const Point& senderLoc);
};

inline void doPacking(cCommBuffer *b, DLBMessage& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, DLBMessage& obj) {obj.parsimUnpack(b);}


#endif // _DLBMESSAGE_M_H_
