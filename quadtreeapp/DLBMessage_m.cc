//
// Generated file, do not edit! Created by opp_msgc 4.2 from DLB/quadtreeapp/DLBMessage.msg.
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
    e->insert(LOC_MSG, "LOC_MSG");
    e->insert(SERVER_MSG, "SERVER_MSG");
    e->insert(CLIENTRANS_MSG, "CLIENTRANS_MSG");
    e->insert(DEBUG_MSG, "DEBUG_MSG");
);

Register_Class(DLBMessage);

DLBMessage::DLBMessage(const char *name, int kind) : cPacket(name,kind)
{
    this->type_var = 0;
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
    this->transferServer_var = other.transferServer_var;
    this->senderKey_var = other.senderKey_var;
}

void DLBMessage::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->type_var);
    doPacking(b,this->transferServer_var);
    doPacking(b,this->senderKey_var);
}

void DLBMessage::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->transferServer_var);
    doUnpacking(b,this->senderKey_var);
}

int DLBMessage::getType() const
{
    return type_var;
}

void DLBMessage::setType(int type)
{
    this->type_var = type;
}

QuadServer& DLBMessage::getTransferServer()
{
    return transferServer_var;
}

void DLBMessage::setTransferServer(const QuadServer& transferServer)
{
    this->transferServer_var = transferServer;
}

OverlayKey& DLBMessage::getSenderKey()
{
    return senderKey_var;
}

void DLBMessage::setSenderKey(const OverlayKey& senderKey)
{
    this->senderKey_var = senderKey;
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
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
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
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
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
        "transferServer",
        "senderKey",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int DLBMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+0;
    if (fieldName[0]=='t' && strcmp(fieldName, "transferServer")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderKey")==0) return base+2;
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
        "QuadServer",
        "OverlayKey",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
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
        case 1: {std::stringstream out; out << pp->getTransferServer(); return out.str();}
        case 2: {std::stringstream out; out << pp->getSenderKey(); return out.str();}
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
        "QuadServer",
        "OverlayKey",
    };
    return (field>=0 && field<3) ? fieldStructNames[field] : NULL;
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
        case 1: return (void *)(&pp->getTransferServer()); break;
        case 2: return (void *)(&pp->getSenderKey()); break;
        default: return NULL;
    }
}


