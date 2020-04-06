#ifndef TEMPLATEUDPSOCKET_H
#define TEMPLATEUDPSOCKET_H

#include <iostream>
#include "platform.h"

//! класс обощенного сокета UDP
template <class T>  class TemplateUdpSocket
{
public:
    enum TTemplateUdpSocket
    {
        E_WAIT_ALL = 0x0040,//0x1, //! ожидание всех пакетов
        E_MSG_PEEK = 0x0002
    }EFLAG;
    TemplateUdpSocket()
    {
    }
    void init(uint16_t port, std::string ip)
    {
        spec_socket.init(port,ip);
    }
    void init(uint16_t port, uint32_t ip)
    {
        spec_socket.init(port,ip);
    }
    bool bindTo()
    {

        return  spec_socket.bindTo();
    }
    int sendTo(uint8_t *buf, uint32_t size)
    {
        return  spec_socket.sendTo(buf,size);
    }
    int sendTo(uint8_t *buf, uint32_t size, uint16_t port, uint32_t addr)
    {
        return spec_socket.sendTo(buf,size,port,addr);
    }
    int sendTo(uint8_t *buf, uint32_t size, uint16_t port)
    {
        return spec_socket.sendTo(buf,size,port);
    }
    bool setTimeout(uint32_t msec)
    {
        return spec_socket.setTimeout(msec);
    }
    //! признак того, что сокет блокируемый
    bool setBlock(bool value)
    {
        return spec_socket.setBlock(value);
    }
    int reciveFrom(uint8_t *buf, uint32_t size, int flag = E_WAIT_ALL)
    {
        return spec_socket.reciveFrom(buf,size,flag);
    }
 private:
    T spec_socket;

};
#if defined(VXWORKS_PLATFORM)
#include "vxUdpSocket.h"
typedef TemplateUdpSocket<VxUdpSocket>       UdpSocket;
#endif

#if defined(QNX_PLATFORM)
#include "qnxUdpSocket.h"
typedef TemplateUdpSocket<QnxUdpSocket>       UdpSocket;
#endif

#if !defined(VXWORKS_PLATFORM) && !defined(QNX_PLATFORM)
#include "./qt/qtUdpSocket.h"
typedef TemplateUdpSocket<QtUdpSocket>       UdpSocket;
#endif

#endif // TEMPLATEUDPSOCKET_H
