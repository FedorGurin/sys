#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <iostream>
#include "platform.h"

//! класс платформ-независмых очереди сообщенйий
template <class T>  class TemplateMsgQueue
{
public:
    TemplateMsgQueue()
    {

    }   

    enum {
    	E_MSG_CREATE = 000400,
    	E_MSG_RDWR = 000002
    };
    void msgCreate(std::string name,uint32_t sizeQ, uint32_t size , int flag = E_MSG_CREATE | E_MSG_RDWR)
    {
        obj.msgCreate(name,sizeQ,size,flag);
    }
    void msgSend(uintptr_t addr,  uint32_t size)
    {
        obj.msgSend(addr,  size);
    }
    bool msgReceive(uintptr_t addr,  uint32_t size)
    {
    	return obj.msgReceive(addr,size);
    }
    void msgSetAttr(int flag = 000200)
    {
       	obj.msgSetAttr(flag);
    }
 private:
    T obj;
};
#if defined(VXWORKS_PLATFORM)
#include "vxMsgQueue.h"
typedef TemplateMsgQueue<VxMsgQueue>       MsgQueue;
#endif

#if defined(QNX_PLATFORM)
#include "qnxMsgQueue.h"
typedef TemplateMsgQueue<QnxMsgQueue>       MsgQueue;
#endif

#if !defined(VXWORKS_PLATFORM) && !defined(QNX_PLATFORM)
#include "./qt/qtMsgQueue.h"
typedef TemplateMsgQueue<QtMsgQueue>       MsgQueue;
#endif

#endif // TEMPLATEUDPSOCKET_H
