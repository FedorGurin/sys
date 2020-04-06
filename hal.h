#ifndef HAL_H
#define HAL_H

#include "platform.h"
#include <string>
#include "connectToEISA.h"
#include "tPrimitives.hpp"
#include "globalNameID.h"
#include "defsHAL.h"
#include "templateUdpSocket.h"
//#include <sys/socket.h>
#ifdef LINUX_PLATFORM
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
//! класс для заполнения таблицы с Arinc
float freq(int fr);

class ConfArTable
{
public:
    ConfArTable(TCh * ch_ = 0)
    {

//       setCh(ch_);
    }

    void setCh(TCh * ch_)
    {
        ch = ch_;
        ar = (TDesAr*)(ch->desData);
        for(int i = 0; i<MAX_ADDR_AR_CYC; i++)
        {
            ar->numIndAddr[i]  = 0;
        }

        ar->numArray    = 0;
        ar->freqArray   = 0;
        ar->curIndArray = 0;
        ar->numAddr     = 0;
    }
    void setChId(TypeChHAL typeCh, uint8_t numCh, uint8_t io)
    {
        ch->typeCh        = typeCh;
        ch->setting.numCh = numCh;
        ch->setting.ioCh  = io;
    }

    void setAddr(TypeNodeHAL typeNode,uint8_t idNode)
    {
        ch->idNode = idNode;
        ch->typeNode = typeNode;
    }
    void setProp(int freq, int rev, int type)
    {
        ar->freq = freq;
        ar->rev = rev;
        ar->type = type;
    }
    ConfArTable& operator<<(uint16_t value)
    {
        if(value == 0)
        {
            ar->numArray++;
            ar->curIndArray = ar->numArray;
            //ar->numAddr[ar->indexAddr]++;
        }
        else if(value > 1000)
        {
            ar->freqArray = value - 1000.;
            //ar->numArray++;
        }else
        {
            int index = -1;
            //! нужно проверить может адрес уже есть в общем списке
            //ar->indAddr[ar->numAddr[ar->indexAddr]][ar->numArray] = value;
            for(int i = 0;i<ar->numAddr; i++)
            {
                if(ar->addr[i] == value)
                {
                    index = i;
                    break;
                }
            }
            ar->numIndAddr[ar->curIndArray]++;
            if(index==-1)
            {
                ar->addr[ar->numAddr++] = value;
                index = ar->numAddr - 1 ;
            }
            ar->indAddr[ar->numIndAddr[ar->curIndArray]-1][ar->curIndArray] = index;

            //ar->addr[] = value;
            //! счетчик элементов в массиве индексов

        }
        return *this;
    }
//    ConfArTable& operator<<(float p)
//    {
//        ar->freqArray   = p;
//        return *this;
//    }
private:
    TCh *ch;
    TDesAr *ar;
};

//! абстрактный класс для работы с аппаратурой(адаптерами ввода-вывода)
template <typename T> class THAL:public T
{
public:
    THAL():T()
    {
        eisaAvailable   = false;
        milAvailable    = false;
        arAvailable     = false;

        eisa = new ConnectToEISA;

        //! обнуление данных
        memset((char*) &confISA     ,0, sizeof(confISA) );
        memset((char*) &chTable     ,0, sizeof(chTable));
        memset((char*) &paramTable  ,0, sizeof(paramTable));
        memset((char*) &ethTable    ,0, sizeof(ethTable));
        memset((char*) &nameTable   ,0, sizeof(nameTable));
        memset((char*) &setup       ,0, sizeof(setup));
        memset((char*) &statusPacket,0, sizeof(statusPacket));
        memset((char*) &packTable   ,0, sizeof(packTable));
        memset((char*) &status      ,0, sizeof(status));
        memset((char*) &ethPortTable,0, sizeof(ethPortTable));
        
        //! тригеры на отключение МКИО по каналам
        trigUpdateOS  = new Trigger<uint8_t>(0);
    }
    //! общая инициализация
    void init();
    //! получить объект
    static THAL<T>* obj();

    //! прочитать данные из адаптеров
    void getFromAdapters();
    //! задать данные в адаптер
    void setToAdapters();
    //! основная функция расчета
    void calculate();

    //! прочитать конфигурацию каналов адаптеров
    void readConfFiles();

    //! загрузка тестовых таблиц
    void loadTestTables();

    //! проверка наличия соединений(периодически вызываемая функция)
    void checkConnections();
    void recCheckConnection();

    //! создание каналов ДПК и МКИО
    void createChAR();
    void createChMIL();

    //! инициализация расширителя EISA
    void initEISA();
    //! инициализация плат ISA
    void initISA();
    //! инициализация плат PCI
    void initPCI();
    //! иницализация системы обеспечения обслуживания
    void initQ();
    //! инициализация подсистемы статусов
    void initStatusSystem();
    //! обновление конфигурационых данных
    void updateVerConf();
    //! загрузка прошивок на целевую машину
    void downloadFirmware();
    //! текущий IP адрес
    uint32_t ipAddr(int numIf)
    {
        TEthIf* nIf = findEthIf(typeCurrentNode,currentNode,numIf);
        if(nIf !=0)
            return inet_addr(nIf->ip);
        return 0;
    }

    uint16_t ethPort(uint32_t id)
    {
        if(id < ethPortTable.numPort)
            return ethPortTable.port[id];

        return 0;
    }

    uint32_t ethIP(int id)
    {
        return inet_addr(ethTable.eth[id].ip);
    }
    TCh* findCh(uint32_t idCh)
    {
        if(idCh<chTable.numCh)
            return &(chTable.ch[idCh]);

        return 0;
    }
    //! найти настрйоки платы ISA
    TAdapterISA* findIsaAdapter(uint16_t typeNode, uint8_t idNode,TypeAdapterISA typeISA)
    {
        for(int i = 0;i < confISA.numAdapters;i++)
        {
            if((confISA.isa[i].typeAdapter == typeISA) && (confISA.isa[i].typeNode == typeNode))
            {
             return &(confISA.isa[i]);
            }
        }
        return 0;
    }
    TEthIf *findEthIf(TypeNodeHAL typeNode, uint32_t idNode, uint8_t numIf)
    {
        for(int i = 0; i < ethTable.numEth;i++)
        {
            if(ethTable.eth[i].fromTypeNode == typeNode &&
               ethTable.eth[i].fromIdNode   == idNode &&
               ethTable.eth[i].numIf    == numIf)
            {
                return &(ethTable.eth[i]);
            }
        }
        return 0;
    }
    TEthIf *findEthIf(TypeNodeHAL fromTypeNode, uint32_t fromIdNode,
                      TypeNodeHAL toTypeNode, uint32_t toIdNode)
    {
        for(int i = 0; i<ethTable.numEth;i++)
        {
            if(ethTable.eth[i].fromTypeNode == fromTypeNode &&
               ethTable.eth[i].fromIdNode   == fromIdNode &&
               ethTable.eth[i].toTypeNode == toTypeNode &&
               ethTable.eth[i].toIdNode   == toIdNode)
            {
                return &(ethTable.eth[i]);
            }
        }
        return 0;
    }
    uint8_t numEthIf(TypeNodeHAL typeNode, uint32_t idNode)
    {
        uint8_t sizeIf = 0;
        for(int i = 0; i < ethTable.numEth;i++)
        {
            if(ethTable.eth[i].fromTypeNode == typeNode &&
               ethTable.eth[i].fromIdNode   == idNode)
            {
                sizeIf++;
            }
        }
        return sizeIf;
    }
    TParamHAL* findParam(uint32_t idParam)
    {
        return &(paramTable.param[idParam]);
    }
    TTablePackParam*  findPack(uint16_t idCh,TParamHAL *param)
    {
        for(int i = 0;i < param->num;i++)
        {
            if(param->idCh[i] == idCh || param->num == 1)
            {
                if(param->bit[i] == 1)
                {
                    packTable.packer[0].hBit = param->idPackCh[i];
                    packTable.packer[0].lBit = param->idPackCh[i];
                    packTable.packer[0].scale = 0.0f;
                    return &(packTable.packer[0]);
                }else
                    return &(packTable.packer[param->idPackCh[i]]);
            }
        }
        return 0;
    }
    //! кол-во плат ISA заданного типа
    TAdapterISA* numISA(TypeAdapterISA type,TypeNodeHAL typeNode = E_NODE_NONE)
    {
        for(int i = 0;i<confISA.numAdapters;i++)
        {
            if(confISA.isa[i].typeNode      == typeCurrentNode &&
               ((confISA.isa[i].idNode      == currentNode &&
                 confISA.isa[i].typeAdapter == type)&& typeNode == E_NODE_NONE) ||
                 confISA.isa[i].typeAdapter == type && confISA.isa[i].typeNode == typeNode && typeNode != E_NODE_NONE)
            {
                return &(confISA.isa[i]);
            }
        }
        return 0;
    }

    //! определение текущего узла
    void addParamToTable(uint32_t idParam, int16_t idCh,    uint16_t idPackCh,     uint16_t idParamCh, bool bit = false  );
    void addParamToTable(uint32_t idParam, int16_t idCh,    uint8_t (*f)(uint8_t), uint8_t bit,uint16_t idParamCh );
    void addPackToTable (uint32_t idParam, uint8_t hBit,    uint8_t lBit,          uint8_t sign, float scale);

    //! подключение к EISA
    ConnectToEISA *eisa;
    //! признак наличия расширителя E-ISA
    volatile bool eisaAvailable;
    //! признак наличия интерфейсов МКИО
    volatile bool milAvailable;
    //! признак наличия интерфейсов Arinc
    volatile bool arAvailable;
    //! идентфикатор узла с EISA
    uint8_t idEisaNode;

    //! параметры настроек
    TSetupHAL  setup;
    //! текущий состояние
    TStatusHAL status;
    //! читаем конфигурацию ISA плат в системе
    TConfISA         confISA;
    TConfPCI         confPCI;
    TTableChHAL      chTable;
    TTableParam      paramTable;
    TTableEth        ethTable;
    TTableEthPort    ethPortTable;
    TTableNode       nameTable;
    TPacketStatus    statusPacket;

    //! таблица упаковок
    TTablePack packTable;
    //! триггер обновления ОС и ММ
    Trigger<uint8_t> *trigUpdateOS;
    //! триггер загрузку конфигурационных файлов
    Trigger<uint8_t> *trigDownConf;
    //! триггер загрузку файлов прошивок
    Trigger<uint8_t> *trigDownFirmware;
    //! триггер загрузку файлов загрузчика
    Trigger<uint8_t> *trigDownBootrom;
    //! текущий тип узла
    TypeNodeHAL  typeCurrentNode;
    //! идентификатор текущего узла
    uint32_t     currentNode;
    //! текущее имя узла
    std::string  currentNameNode;
    //! для совместимости со старыми версиями
    EGlobalNodeID currentGlobalTypeNode;
private:
    static THAL<T> *hal;
    //! сокеты для определения признака соеднения
    //! сокет для передачи данных в узлы
    UdpSocket *udpSend;
    //! сокет для получения данных от узлов
    UdpSocket *udpRec;
};
#ifdef VXWORKS_PLATFORM
    #include "vxHSL.h"
    typedef THAL<VxHSL> HAL;
    template <typename T> T*  tHAL()
    {
        return T::obj();
    }
#else
    #ifdef QNX_PLATFORM
        #include "qnxHSL.h"
        typedef THAL<QnxHSL> HAL;
        template <typename T> T*  tHAL()
        {
            return T::obj();
        }
    #else
        #include "./qt/qtHSL.h"
        typedef THAL<QtHSL> HAL;
    #endif
#endif

/*HAL*  tHAL()
{
    return HAL::obj();
}*/
#endif // HAL_H
