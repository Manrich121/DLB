//
// Generated file, do not edit! Created by opp_msgc 4.2 from DLB/DLBMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "DLBMessage_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("MessageType");
    if (!e) enums.getInstance()->add(e = new cEnum("MessageType"));
    e->insert(CLIENTSIZE_MSG, "CLIENTSIZE_MSG");
    e->insert(SERVER_MSG, "SERVER_MSG");
    e->insert(CLIENTRANS_MSG, "CLIENTRANS_MSG");
    e->insert(REQKEY_MSG, "REQKEY_MSG");
    e->insert(RETSERV_MSG, "RETSERV_MSG");
    e->insert(FREEME_MSG, "FREEME_MSG");
    e->insert(NEIGH_REQ, "NEIGH_REQ");
    e->insert(NEIGH_A_R, "NEIGH_A_R");
    e->insert(DEBUG_MSG, "DEBUG_MSG");
    e->insert(CLIENTT_ACK, "CLIENTT_ACK");
);

Register_Class(DLBMessage);

DLBMessage::DLBMessage(const char *name, int kind) : cPacket(name,kind)
{
    this->type_var = 0;
    this->clientSize_var = 0;
}

DLBMessage::DLBMessage(const DLBMessage& other) : cPacket(other)
{
    copy(other);
}

DLBMessage::~DLBMessage()
{
}

DLBMessage& DLBMessage::operator=(const DLBMessage& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void DLBMessage::copy(const DLBMessage& other)
{
    this->type_var = other.type_var;
    this->senderKey_var = other.senderKey_var;
    this->clients_var = other.clients_var;
    this->removeKey_var = other.removeKey_var;
    this->quadServer_var = other.quadServer_var;
    this->rects_var = other.rects_var;
    this->voroServer_var = other.voroServer_var;
    this->senderLoc_var = other.senderLoc_var;
    this->clientSize_var = other.clientSize_var;
    this->myNeighs_var = other.myNeighs_var;
}

void DLBMessage::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->type_var);
    doPacking(b,this->senderKey_var);
    doPacking(b,this->clients_var);
    doPacking(b,this->removeKey_var);
    doPacking(b,this->quadServer_var);
    doPacking(b,this->rects_var);
    doPacking(b,this->voroServer_var);
    doPacking(b,this->senderLoc_var);
    doPacking(b,this->clientSize_var);
    doPacking(b,this->myNeighs_var);
}

void DLBMessage::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->senderKey_var);
    doUnpacking(b,this->clients_var);
    doUnpacking(b,this->removeKey_var);
    doUnpacking(b,this->quadServer_var);
    doUnpacking(b,this->rects_var);
    doUnpacking(b,this->voroServer_var);
    doUnpacking(b,this->senderLoc_var);
    doUnpacking(b,this->clientSize_var);
    doUnpacking(b,this->myNeighs_var);
}

int DLBMessage::getType() const
{
    return type_var;
}

void DLBMessage::setType(int type)
{
    this->type_var = type;
}

OverlayKey& DLBMessage::getSenderKey()
{
    return senderKey_var;
}

void DLBMessage::setSenderKey(const OverlayKey& senderKey)
{
    this->senderKey_var = senderKey;
}

clientVect& DLBMessage::getClients()
{
    return clients_var;
}

void DLBMessage::setClients(const clientVect& clients)
{
    this->clients_var = clients;
}

OverlayKey& DLBMessage::getRemoveKey()
{
    return removeKey_var;
}

void DLBMessage::setRemoveKey(const OverlayKey& removeKey)
{
    this->removeKey_var = removeKey;
}

QuadServer& DLBMessage::getQuadServer()
{
    return quadServer_var;
}

void DLBMessage::setQuadServer(const QuadServer& quadServer)
{
    this->quadServer_var = quadServer;
}

rectVect& DLBMessage::getRects()
{
    return rects_var;
}

void DLBMessage::setRects(const rectVect& rects)
{
    this->rects_var = rects;
}

VoroServer& DLBMessage::getVoroServer()
{
    return voroServer_var;
}

void DLBMessage::setVoroServer(const VoroServer& voroServer)
{
    this->voroServer_var = voroServer;
}

Point& DLBMessage::getSenderLoc()
{
    return senderLoc_var;
}

void DLBMessage::setSenderLoc(const Point& senderLoc)
{
    this->senderLoc_var = senderLoc;
}

int DLBMessage::getClientSize() const
{
    return clientSize_var;
}

void DLBMessage::setClientSize(int clientSize)
{
    this->clientSize_var = clientSize;
}

keySet& DLBMessage::getMyNeighs()
{
    return myNeighs_var;
}

void DLBMessage::setMyNeighs(const keySet& myNeighs)
{
    this->myNeighs_var = myNeighs;
}

class DLBMessageDescriptor : public cClassDescriptor
{
  public:
    DLBMessageDescriptor();
    virtual ~DLBMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(DLBMessageDescriptor);

DLBMessageDescriptor::DLBMessageDescriptor() : cClassDescriptor("DLBMessage", "cPacket")
{
}

DLBMessageDescriptor::~DLBMessageDescriptor()
{
}

bool DLBMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<DLBMessage *>(obj)!=NULL;
}

const char *DLBMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int DLBMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 10+basedesc->getFieldCount(object) : 10;
}

unsigned int DLBMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<10) ? fieldTypeFlags[field] : 0;
}

const char *DLBMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "type",
        "senderKey",
        "clients",
        "removeKey",
        "quadServer",
        "rects",
        "voroServer",
        "senderLoc",
        "clientSize",
        "myNeighs",
    };
    return (field>=0 && field<10) ? fieldNames[field] : NULL;
}

int DLBMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderKey")==0) return base+1;
    if (fieldName[0]=='c' && strcmp(fieldName, "clients")==0) return base+2;
    if (fieldName[0]=='r' && strcmp(fieldName, "removeKey")==0) return base+3;
    if (fieldName[0]=='q' && strcmp(fieldName, "quadServer")==0) return base+4;
    if (fieldName[0]=='r' && strcmp(fieldName, "rects")==0) return base+5;
    if (fieldName[0]=='v' && strcmp(fieldName, "voroServer")==0) return base+6;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderLoc")==0) return base+7;
    if (fieldName[0]=='c' && strcmp(fieldName, "clientSize")==0) return base+8;
    if (fieldName[0]=='m' && strcmp(fieldName, "myNeighs")==0) return base+9;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *DLBMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "OverlayKey",
        "clientVect",
        "OverlayKey",
        "QuadServer",
        "rectVect",
        "VoroServer",
        "Point",
        "int",
        "keySet",
    };
    return (field>=0 && field<10) ? fieldTypeStrings[field] : NULL;
}

const char *DLBMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "MessageType";
            return NULL;
        default: return NULL;
    }
}

int DLBMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    DLBMessage *pp = (DLBMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string DLBMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    DLBMessage *pp = (DLBMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getType());
        case 1: {std::stringstream out; out << pp->getSenderKey(); return out.str();}
        case 2: {std::stringstream out; out << pp->getClients(); return out.str();}
        case 3: {std::stringstream out; out << pp->getRemoveKey(); return out.str();}
        case 4: {std::stringstream out; out << pp->getQuadServer(); return out.str();}
        case 5: {std::stringstream out; out << pp->getRects(); return out.str();}
        case 6: {std::stringstream out; out << pp->getVoroServer(); return out.str();}
        case 7: {std::stringstream out; out << pp->getSenderLoc(); return out.str();}
        case 8: return long2string(pp->getClientSize());
        case 9: {std::stringstream out; out << pp->getMyNeighs(); return out.str();}
        default: return "";
    }
}

bool DLBMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    DLBMessage *pp = (DLBMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setType(string2long(value)); return true;
        case 8: pp->setClientSize(string2long(value)); return true;
        default: return false;
    }
}

const char *DLBMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        "OverlayKey",
        "clientVect",
        "OverlayKey",
        "QuadServer",
        "rectVect",
        "VoroServer",
        "Point",
        NULL,
        "keySet",
    };
    return (field>=0 && field<10) ? fieldStructNames[field] : NULL;
}

void *DLBMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    DLBMessage *pp = (DLBMessage *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getSenderKey()); break;
        case 2: return (void *)(&pp->getClients()); break;
        case 3: return (void *)(&pp->getRemoveKey()); break;
        case 4: return (void *)(&pp->getQuadServer()); break;
        case 5: return (void *)(&pp->getRects()); break;
        case 6: return (void *)(&pp->getVoroServer()); break;
        case 7: return (void *)(&pp->getSenderLoc()); break;
        case 9: return (void *)(&pp->getMyNeighs()); break;
        default: return NULL;
    }
}


