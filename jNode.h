#ifndef JNODE_H
#define JNODE_H

#include "ICalculateElement.h"
#include "templateUdpSocket.h"
#include <vector>

#define MAX_BUF 4096
#define MAX_LIM_BUF 1400
////! список приемников данных
typedef struct TUdpReciveList_
{
    //! тип узла
    uint8_t type;
    //! идентификатор узла
    uint8_t idNode;
    //! сокет для приема данных
    //VxTemplateUdpSocket *udpSocketRec;
}TUdpReciveList;

//! заголовок пакета
typedef struct TUdpJNodeHead_
{
    //! тип пакета(канальная запись - 0, 1 - наличие расширителей EISA, 2 - all data)
    uint8_t type;
    //! размер переданого буфера
    uint16_t sizeBuf;
}TUdpJNodeHead;

//! тело пакета
typedef struct TUdpJNodeCh_
{
    //! индекс канала
    uint16_t indexCh;
    //!
    uint64_t tics;
    //!
    float step;
    //! буфер в зависимости типа канала
    uint8_t data[];
}TUdpJNodeCh;

//! пакет для передачи данных между узлами
typedef struct TUdpJNodePacket_
{
    //! заголовок пакета
    TUdpJNodeHead head;
    //! буфер с данными
    uint8_t buffer[MAX_BUF];

}TUdpJNodePacket;

//! класс для узлами УЦВС
class  JNode:public ICalculateElement
{
public:
    JNode(uint32_t idClass);

    //! обобщенный интерфейс
    virtual void init();
    virtual void calculate();
    virtual void finite(){}
    void sendDataToNodes();
    void recDataFromNodes();
    void parserRecData();
    void packetFull(uint16_t *offset, unsigned long ip, uint8_t type = 0);
private:
    //! список сокетов для приема
    std::vector<TUdpReciveList* > listUdpRec;
    //! сокет для передачи данных в узлы
    UdpSocket *udpSend;
    //! сокет для получения данных от узлов
    UdpSocket *udpRec;
    //! пакет для отправки
    TUdpJNodePacket sendPacket;
    TUdpJNodePacket recPacket;
    TUdpJNodeHead recHead;
    TTimerIC timerCh;

};

#endif
