#include "hal.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>

#include "layerSpecRTM.h"
#include "layerMIL.h"
#include "layerRK.h"
#include "layerAnalog.h"
#include "chdev.h"

#include <ctime>
#ifdef VXWORKS_PLATFORM
#include <net/utils/ifconfig.h>
#include <Ar16t16rApi.h>
#include <MonNik_api.h>
#include <sockLib.h>
#include "tftpLib.h"
#include "ioLib.h"
#include "rebootLib.h"
#include "usrFsLib.h"


#include <ipProto.h>
#include <net/if.h>
#include "inetLib.h"
#include "ioLib.h"
#include <sys/ioctl.h>
#endif


#include "glHAL.hpp"
#include "arServ.h"
//#include "servUpdate.h"
//#include "threadManager.h"



#ifdef VXWORKS_PLATFORM

HAL* HAL::hal = 0;
HAL* HAL::obj() {
    if (hal == 0)
        hal = new HAL();

    return hal;
}
#else
template<> HAL* HAL::hal = 0;
template<> HAL* HAL::obj() {
    if (hal == 0)
        hal = new HAL();

    return hal;
}
#endif
float freqHz(int fr)
{
    fr+=1000;
    return fr;
}
//! базовая директория
#ifdef VXWORKS_PLATFORM
const std::string baseFolder = "/ata0a/";
#endif
#ifdef QNX_PLATFORM
const std::string baseFolder = "/home/";
#else
const std::string baseFolder = "./ata0a/";
#endif
//! имена файлов
const std::string nfConfISA     = "confISA.bin"      ;
const std::string nfChTable     = "chTable.bin"      ;
const std::string nfParamTable  = "paramTable.bin"   ;
const std::string nfEthTable    = "ethTable.bin"     ;
const std::string nfNameTable   = "nameTable.bin"    ;
const std::string nfEthPortTable= "ethPortTable.bin" ;
const std::string nfPackTable   = "packTable.bin"    ;
const std::string nfVerFile     = "ver.txt"          ;
//! пути к файлам
const std::string pathToConf        = baseFolder + "conf/"          ;
const std::string pathToConfISA     = pathToConf + nfConfISA      ;
const std::string pathToChTable     = pathToConf + nfChTable      ;
const std::string pathToParamTable  = pathToConf + nfParamTable   ;
const std::string pathToEthTable    = pathToConf + nfEthTable     ;
const std::string pathToNameTable   = pathToConf + nfNameTable    ;
const std::string pathToEthPortTable= pathToConf + nfEthPortTable ;
const std::string pathToPackTable   = pathToConf + nfPackTable     ;
const std::string pathToVerFile     = pathToConf + nfVerFile      ;

template <> void HAL::initQ()
{
    //! создаем поток
    std::string pthrName = "pthServUpdate";// + std::(ar->freqArray);
//    Thread* thServUpdate = new Thread(pthrName,
//                                      new ServUpdate(ID_SERV_UPDATE,1.0/12.5));
//    thServUpdate->create();
}
//! создание директорий для файлов модели

/*int HAL::uploadFile(std::string localPath, std::string nameFile)
{

}*/
template<> void HAL::recCheckConnection()
{
    int bytes = -1;

    do
    {
        bytes = -1;
        memset((void*)&statusPacket,0,sizeof(statusPacket));
        //! считываем заголовок
        bytes = udpRec->reciveFrom((uint8_t*)&statusPacket, sizeof(statusPacket));

        //! определяем ip адрес этого узла
        TEthIf *eth= HAL::obj()->findEthIf(HAL::obj()->typeCurrentNode,
                                           HAL::obj()->currentNode,
                                           statusPacket.typeNode,
                                           statusPacket.idNode);
        if(eth !=0 && statusPacket.status == 0)
            eth->isConnect = 2;
    }while(bytes>0);
}
template <> void HAL::checkConnections()
{
    statusPacket.typeNode = HAL::obj()->typeCurrentNode;
    statusPacket.idNode   = HAL::obj()->currentNode;
    statusPacket.status   = 0;
    //! кол-во сетевых интерфейсов с другими узлами
    uint8_t numIf = HAL::obj()->numEthIf(HAL::obj()->typeCurrentNode,
                                         HAL::obj()->currentNode);
    //! цикл по доступным типам узлов
    for(int i = 0; i< numIf; i++)
    {
        //! выделяем сетевой интерфейс с каким ПВ связан
        TEthIf *eth = HAL::obj()->findEthIf(HAL::obj()->typeCurrentNode,
                                            HAL::obj()->currentNode,i);

        if(eth == 0)
            continue;
        //! определяем ip адрес этого узла
        TEthIf *eth1= HAL::obj()->findEthIf(eth->toTypeNode,
                                            eth->toIdNode,
                                            HAL::obj()->typeCurrentNode,
                                            HAL::obj()->currentNode);

        if(eth1 == 0)
            continue;
        //! проверяю соединение для данных интерфейсов(пинг или не пинг!?)
        int bytes = udpSend->sendTo((uint8_t*)&statusPacket,
                        sizeof(statusPacket),
                        HAL::obj()->ethPort(ETH_PORT_REC_CHECK_HAL), eth1->ip_int);
        if(bytes == -1)
            eth1->isConnect = 0;
        else
            eth1->isConnect = 1;
    }
    //! проверить пришли ли ответы
    recCheckConnection();
}


template <> void HAL::initStatusSystem()
{
    //! получение данных от других узлов
    udpRec = new UdpSocket;
    udpRec->init(HAL::obj()->ethPort(ETH_PORT_REC_CHECK_HAL),"");
    udpRec->bindTo();
    udpRec->setBlock(false);

    //! сокет для отправки в другие узлы
    udpSend = new UdpSocket();
    udpSend->init(HAL::obj()->ethPort(ETH_PORT_REC_CHECK_HAL),"");
}
template <> void HAL::calculate()
{
}
template <> void HAL::initEISA() {
    std::cout << "========================================" << std::endl;
    std::cout << "HAL: Scaning EISA ..." << std::endl;
    eisa->startSearch();
    eisaAvailable = eisa->checkAvailable();
    if (eisaAvailable == true)
        std::cout << "EISA is connected" << std::endl;
    else
        std::cout << "EISA not found" << std::endl;
}
template <>void HAL::initISA() {
    std::cout << "========================================" << std::endl;
    std::cout << "HAL: Scaning ISA adapters..." << std::endl;

    //! инициализация библиотеки работы с РК
    LayerRK::obj();
    //! инициализация библиотеки работы с ЦАП
    //LayerDAC::obj();
    //! инициализация библиотеки работы с АЦП
    //LayerADC::obj();
    //! иммитация аналоговых сигналов(ИМ сопотивлений, потенциометра и т.д)
    LayerAnalog::obj();
}
//extern bool InitMonitorMil(void);
//extern void InitRegimeMonitor(uint8_t indexAdapter);
template<> void HAL::initPCI() {

#ifndef VXWORKS_SIM
    std::cout << "========================================" << std::endl;
    std::cout << "HAL: Scaning PCI adapters ..." << std::endl;
    std::cout << std::setw(30) << std::left << "HAL: Detected AR16T16R .... ";
    if (LayerConfAr::initApi() == false)
        std::cout << " Channels" << " [NOT FOUND]" << std::endl;
    else
        std::cout << LayerConfAr::maxNumCh() << " Channels" << " [READY]" << std::endl;
    //! инициализация адаптеров МКИО
    std::cout << std::setw(30) << std::left << "HAL: Detected MOM4 .... ";
    milAvailable = LayerConfMIL::initApi();
    if (milAvailable == false)
        std::cout << " 0 Channels" << " [NOT FOUND]" << std::endl;
    else
        std::cout << LayerConfMIL::maxNumCh() << " Channels" << " [READY]" << std::endl;

    //! инициализация 4 канальных мониторов
    for(int i = 0; i<nameTable.numNode; i++)
    {
        for(int j = 0;j<confPCI.numMil; j++)
        {
            if(nameTable.node[i].typeNode == confPCI.mil[j].typeNode &&
               nameTable.node[i].idNode == confPCI.mil[j].idNode)
            {
                for(int k = 0; k < confPCI.mil[j].numAdapterRegMon; k++)
                {
                    //! каналы МКИО
                    int index;
//                    int index = confPCI.mil[j].numAdapterRegOU + k;
//                    InitRegimeMonitor(index);
                    std::cout << std::setw(30) << std::left << "HAL: adapter "
                              << index <<" MOM4 set Monitor regime " <<std::endl;
                }
            }
        }
    }
#endif
}
template<>void HAL::readConfFiles()
{
    std::cout << "========================================" << std::endl;
    std::cout << "HAL: Loading Configuration data" << std::endl;
    if(checkConfFile(pathToConfISA, sizeof(confISA)) == false)
        uploadFile(pathToConf,nfConfISA);
    readConfFromFile(pathToConfISA, (uintptr_t *) &confISA, sizeof(confISA));
        
    if(checkConfFile(pathToChTable , sizeof(chTable)) == false)
        uploadFile(pathToConf,nfChTable);
    readConfFromFile(pathToChTable, (uintptr_t *) &chTable, sizeof(chTable));
    
    if(checkConfFile(pathToParamTable, sizeof(paramTable)) == false)
        uploadFile(pathToConf,nfParamTable);
    readConfFromFile(pathToParamTable, (uintptr_t *) &paramTable,  sizeof(paramTable));
       
    if(checkConfFile(pathToEthTable, sizeof(ethTable)) == false)
        uploadFile(pathToConf,nfEthTable);
    readConfFromFile(pathToEthTable, (uintptr_t *) &ethTable,  sizeof(ethTable));
       
    if(checkConfFile(pathToNameTable, sizeof(nameTable)) == false)
        uploadFile(pathToConf,nfNameTable);
    readConfFromFile(pathToNameTable, (uintptr_t *) &nameTable, sizeof(nameTable));
    
    if(checkConfFile(pathToVerFile) == false)
        uploadFile(pathToConf,nfVerFile);
    readConfFromFile(pathToVerFile, (uintptr_t *) status.verModel, sizeof(status.verModel));
    
    if(checkConfFile(pathToEthPortTable, sizeof(ethPortTable)) == false)
        uploadFile(pathToConf,nfEthPortTable);
    readConfFromFile(pathToEthPortTable, (uintptr_t *) &ethPortTable, sizeof(ethPortTable));
    
    if(checkConfFile(pathToPackTable, sizeof(packTable)) == false)
        uploadFile(pathToConf,nfPackTable);
    readConfFromFile(pathToPackTable, (uintptr_t *) &packTable,       sizeof(packTable));}


template<> void HAL::createChMIL()
{
    TDesMIL *mil = 0;
    TTableChHAL* table = &(HAL::obj()->chTable);
    for (int i = 0; i < table->numCh; i++)
    {
        TCh *ch = &(HAL::obj()->chTable.ch[i]);
        if (ch->typeNode == HAL::obj()->typeCurrentNode && ch->idNode == HAL::obj()->currentNode)
        {
            if (ch->typeCh == E_CH_MIL)
            {
                mil = (TDesMIL*) (ch->desData);
                LayerOuMIL::enable(ch->setting.numCh, mil->addr, ch->setting.numAdapter, 1);
                TDesMIL *mil = (TDesMIL*) ch->desData;
                LayerOuMIL::writeToAdapter(ch->setting.numAdapter,ch->setting.numCh, mil->addr, mil->subAddr, mil->s.ui32);
                LayerOuMIL::enable(ch->setting.numCh, mil->addr, ch->setting.numAdapter, 0);
                //LayerOuMIL::clearSubAddr(ch->setting.numCh, mil->addr, ch->setting.numAdapter, 1);
            }
        }
    }
}
template<> void HAL::createChAR() {
    TDesAr *ar = 0;

    TTableChHAL* table = &(HAL::obj()->chTable);

    for (int i = 0; i < table->numCh; i++)
    {
        TCh *ch = &(HAL::obj()->chTable.ch[i]);
        if (ch->typeNode == typeCurrentNode && ch->idNode == currentNode)
        {
            if (ch->typeCh == E_CH_AR)
            {
                ar = (TDesAr*) (ch->desData);
                if(ar->freqArray > 0)
                {
//                    //! создаем поток
//                    ICalculateElement *arServ = new ArServ(ID_AR_SERV,i,1./ar->freqArray);
//                    std::string pthrName = "pthArFreq";// + std::(ar->freqArray);
//                    Thread* thArHal = new Thread(pthrName,arServ);
//                    thArHal->create();
                }
                uint8_t bufAddr[MAX_ADR_AR];
                uint8_t numAddr = 0;

                for(int j = 0;j<ar->numIndAddr[ar->curIndArray];j++)
                {
                    bufAddr[j] = ar->addr[ar->indAddr[j][ar->curIndArray]];
                    numAddr++;
                }
                //! инициализируем канал
                LayerArinc::initCh(ch->setting.numCh, bufAddr, numAddr, ch->setting.ioCh, ar->freq, ar->type, ar->rev);
                //if(ch->setting.ioCh == 0)
               // {
                    ioctlHAL(i,  IO_SWITCH_ON_OFF,1);
                    ch->setting.activeLocal = 1;
                    ar->active = 1;
               // }
            }
        }
    }
}
template <> void HAL::setToAdapters()
{
    HAL* t = HAL::obj();
    TCh* ch = 0;
    //! записываю данные, если они принадлежат данному узлу
    for (int i = 0; i < t->chTable.numCh; i++)
    {
        ch = &(t->chTable.ch[i]);
        if (ch->idNode == currentNode && ch->typeNode == typeCurrentNode &&
            ch->setting.ioCh == 1 &&
            ch->setting.readyToWrite == 1 &&
            ch->setting.blockWrite == 0)
        {
            ch->setting.readyToWrite = 0;
            //! аппаратная запись на данном узле
            switch (ch->typeCh)
            {
            case E_CH_MIL:
            {
                TDesMIL *mil = (TDesMIL*) ch->desData;
                if(mil->active!= ch->setting.activeLocal)
                {
                    ioctlHAL(i,  IO_SWITCH_ON_OFF,mil->active);
                    ch->setting.activeLocal = mil->active;
                }
                
                LayerOuMIL::writeToAdapter(ch->setting.numAdapter,
                        ch->setting.numCh, mil->addr, mil->subAddr, mil->s.ui32);
                break;
            }
            case E_CH_AR: {
                TDesAr *ar = (TDesAr*) ch->desData;

                //! если пришла команда на включение
                if(ar->active!= ch->setting.activeLocal)
                {
                    ioctlHAL(i,  IO_SWITCH_ON_OFF,ar->active);
                    ch->setting.activeLocal = ar->active;
                }

                for(int i = 0;i<ar->numIndAddr[ar->curIndArray];i++)
                {
                    LayerArinc::writeTo(ch->setting.numCh, ch->setting.ioCh,
                            ar->value[ar->indAddr[i][ar->curIndArray]], ar->addr[ar->indAddr[i][ar->curIndArray]], i);
                }

//                for (int j = 0; j < ar->numAddr[ar->indexAddr]; j++)
//                {
//                    LayerArinc::writeTo(ch->setting.numCh, ch->setting.ioCh,
//                            ar->value[j], ar->addr[j][ar->indexAddr], j);
//                }
                break;
            }
            case E_CH_RK: {
                //LayerRK::set()
                break;
            }

            };

        } else if (eisaAvailable == true && ch->typeNode == E_NODE_EISA)
        {
            if (ch->typeCh == E_CH_RK) {
                //TAdapterISA * isa = findIsaAdapter(E_NODE_EISA, 1, E_A_TRREL48);
                //! создаем запрос по указаному каналу
                //eisa->sendRequest(i);
                //eisa->writeTo(isa->baseAddr[ch->setting.numAdapter], ,false);

            }
            //! отправляем данные в EISA
            //eisa->writeTo(0x120,(char*)&value0, 4,false);

        } else if (ch->typeNode == E_NODE_EISA) {
            //! определяем узел который имеет доступ к узлу EISA

        } else {
            //! отправляю данные к узлу, который владеет данными каналами
            //            if(ch->setting.ready == 1)
            //            {
            //             //   addToReq(ch);
            //                ch->setting.ready = 0;
            //            }
        }
    }
    //! запись сразу блоком
    LayerRK::obj()->writeToDevice();
    LayerAnalog::obj()->writeToDevice();
}
template <> void HAL::getFromAdapters() {
    HAL* t = HAL::obj();
    TCh* ch = 0;
    //! записываю данные, если они принадлежат данному узлу
    for (int i = 0; i < t->chTable.numCh; i++)
    {
        ch = &(t->chTable.ch[i]);
        if (ch->idNode == currentNode &&
            ch->typeNode == typeCurrentNode  &&
            ch->setting.ioCh == 0 &&
            ch->setting.blockRead == 0) {
            //! аппаратная запись на данном узле
            switch (ch->typeCh) {
            case E_CH_MIL: {
                TDesMIL *mil = (TDesMIL*) ch->desData;
                if(mil->typeTrans == MON)
                    LayerMonMIL::readFromAdapter(ch->setting.numAdapter, ch->setting.numCh, mil->addr, mil->subAddr, mil->s.ui32);
                else
                    LayerOuMIL::readFromAdapter(ch->setting.numAdapter, ch->setting.numCh, mil->addr, mil->subAddr, mil->s.ui32);
                break;
            }
            case E_CH_AR: {
                TDesAr *ar = (TDesAr*) ch->desData;
                for(int i = 0;i<ar->numIndAddr[ar->curIndArray];i++)
                {

                    ar->value[ar->indAddr[i][ar->curIndArray]]= LayerArinc::readFrom(ch->setting.numCh, ch->setting.ioCh, ar->addr[ar->indAddr[i][ar->curIndArray]], ar->addr[ar->indAddr[i][ar->curIndArray]]);
                    //writeTo(ch->setting.numCh, ch->setting.ioCh,        , ar->addr[ar->indAddr[i][ar->curIndArray]], j);
                }

                break;
            }
            case E_CH_RK:
            {

                break;
            }

            };

        } else if (eisaAvailable == true && ch->typeNode == E_NODE_EISA) {
            //! отправляем данные в EISA

            //eisa->
            switch (ch->typeCh) {
            case E_CH_RK: {

                //                        uint16_t index = findIsaAdapter();
                //                        eisa->writeTo(ch->setting.numAdapter);
                break;
            }

            };

        } else if (ch->typeNode == E_NODE_EISA) {
            //! определяем узел который имеет доступ к узлу EISA

        } else {
            //! отправляю данные к узлу, который владеет данными каналами

        }
    }
    //! чтение сразу блоком
    LayerRK::obj()->readFromDevice();
    LayerAnalog::obj()->readFromDevice();
}
template<> void HAL::loadTestTables() {
    ConfArTable arTable;
    HAL* t = HAL::obj();
    TDesMIL *mil     = 0;
    //TDesAr *ar       = 0;
    TDesIR *ir       = 0;
    TDesDAC *dac     = 0;
    TDesGen_U *gen_u = 0;
    TDesITP *itp     = 0;

 ////////////////
    //! таблица с названиями
    t->nameTable.numNode = 3;
    ///
    t->nameTable.node[0].typeNode = E_NODE_CV;
    t->nameTable.node[0].idNode = 1;
    strcpy(t->nameTable.node[0].name, "BREO");
    strcpy(t->nameTable.node[0].mac, "00-1B-21-A6-CA-B0");

    t->nameTable.node[1].typeNode = E_NODE_PV;
    t->nameTable.node[1].idNode = 1;
    strcpy(t->nameTable.node[1].name, "OVO");
    strcpy(t->nameTable.node[1].mac, "00-1B-21-A6-C8-F0");

    t->nameTable.node[2].typeNode = E_NODE_PV;
    t->nameTable.node[2].idNode = 2;
    strcpy(t->nameTable.node[2].name, "RP");
    strcpy(t->nameTable.node[2].mac, "00-0B-AB-D5-EE-6F");
	
	
	//! список портов для обмена данными по ethernet
    t->ethPortTable.numPort = 6;
    t->ethPortTable.port[ETH_PORT_SEND_VIS_SIVVO]   = 7030;
    t->ethPortTable.port[ETH_PORT_REC_RLF_SIVVO]    = 5510;
    t->ethPortTable.port[ETH_PORT_SEND_IND_SIVVO]   = 7016;
    t->ethPortTable.port[ETH_PORT_REC_JOY]          = 8881;
    t->ethPortTable.port[ETH_PORT_REC_MPPM]         = 9999;
    t->ethPortTable.port[ETH_PORT_JNODE]    		= 7070;
  
	
	 //! таблица Ethernet каналов
    t->ethTable.numEth = ETH_IP_PW_CON_5 + 1;

    t->ethTable.eth[ETH_IP_ARM_BREO].fromTypeNode = E_NODE_CV;
    t->ethTable.eth[ETH_IP_ARM_BREO].fromIdNode = 1;
    t->ethTable.eth[ETH_IP_ARM_BREO].numIf = 0;
    t->ethTable.eth[ETH_IP_ARM_BREO].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_ARM_BREO].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_ARM_BREO].ip, "192.168.226.101");
    strcpy(t->ethTable.eth[ETH_IP_ARM_BREO].nameIf, "gei0");

    t->ethTable.eth[ETH_IP_ARM_OVO].fromTypeNode = E_NODE_PV;
    t->ethTable.eth[ETH_IP_ARM_OVO].fromIdNode = 1;
    t->ethTable.eth[ETH_IP_ARM_OVO].numIf = 0;
    t->ethTable.eth[ETH_IP_ARM_OVO].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_ARM_OVO].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_ARM_OVO].ip, "192.168.226.102");
    strcpy(t->ethTable.eth[ETH_IP_ARM_OVO].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_ARM_RP].fromTypeNode = E_NODE_PV;
    t->ethTable.eth[ETH_IP_ARM_RP].fromIdNode = 2;
    t->ethTable.eth[ETH_IP_ARM_RP].numIf = 0;
    t->ethTable.eth[ETH_IP_ARM_RP].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_ARM_RP].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_ARM_RP].ip, "192.168.226.103");
    strcpy(t->ethTable.eth[ETH_IP_ARM_RP].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_SRMP].fromTypeNode = E_NODE_LMI;
    t->ethTable.eth[ETH_IP_SRMP].fromIdNode = 1;
    t->ethTable.eth[ETH_IP_SRMP].numIf = 0;
    t->ethTable.eth[ETH_IP_SRMP].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_SRMP].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_SRMP].ip, "192.168.226.160");
    strcpy(t->ethTable.eth[ETH_IP_SRMP].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_SIVVO].fromTypeNode = E_NODE_KGO;
    t->ethTable.eth[ETH_IP_SIVVO].fromIdNode = 1;
    t->ethTable.eth[ETH_IP_SIVVO].numIf = 0;
    t->ethTable.eth[ETH_IP_SIVVO].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_SIVVO].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_SIVVO].ip, "192.168.226.10");
    strcpy(t->ethTable.eth[ETH_IP_SIVVO].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_PW_CON_1].fromTypeNode = E_NODE_PV;
    t->ethTable.eth[ETH_IP_PW_CON_1].fromIdNode = 10;
    t->ethTable.eth[ETH_IP_PW_CON_1].numIf = 0;
    t->ethTable.eth[ETH_IP_PW_CON_1].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_PW_CON_1].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_1].ip, "192.168.226.151");
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_1].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_PW_CON_2].fromTypeNode = E_NODE_PV;
    t->ethTable.eth[ETH_IP_PW_CON_2].fromIdNode = 10;
    t->ethTable.eth[ETH_IP_PW_CON_2].numIf = 0;
    t->ethTable.eth[ETH_IP_PW_CON_2].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_PW_CON_2].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_2].ip, "192.168.226.152");
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_2].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_PW_CON_3].fromTypeNode = E_NODE_PV;
    t->ethTable.eth[ETH_IP_PW_CON_3].fromIdNode = 10;
    t->ethTable.eth[ETH_IP_PW_CON_3].numIf = 0;
    t->ethTable.eth[ETH_IP_PW_CON_3].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_PW_CON_3].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_3].ip, "192.168.226.153");
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_3].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_PW_CON_4].fromTypeNode = E_NODE_PV;
    t->ethTable.eth[ETH_IP_PW_CON_4].fromIdNode = 10;
    t->ethTable.eth[ETH_IP_PW_CON_4].numIf = 0;
    t->ethTable.eth[ETH_IP_PW_CON_4].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_PW_CON_4].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_4].ip, "192.168.226.154");
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_4].nameIf, "gei0");
	
	t->ethTable.eth[ETH_IP_PW_CON_5].fromTypeNode = E_NODE_PV;
    t->ethTable.eth[ETH_IP_PW_CON_5].fromIdNode = 10;
    t->ethTable.eth[ETH_IP_PW_CON_5].numIf = 0;
    t->ethTable.eth[ETH_IP_PW_CON_5].toTypeNode = E_NODE_NONE;
    t->ethTable.eth[ETH_IP_PW_CON_5].toIdNode = 0;
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_5].ip, "192.168.226.155");
    strcpy(t->ethTable.eth[ETH_IP_PW_CON_5].nameIf, "gei0");
	
     //! кол-во адаптеров
    t->confISA.numAdapters = 6;
    //-----------------------------------------------------
    t->confISA.isa[0].typeNode = E_NODE_PV;
    t->confISA.isa[0].idNode = 0;
    t->confISA.isa[0].typeAdapter = E_A_GEN;
    t->confISA.isa[0].numBaseAddr = 2;
    t->confISA.isa[0].baseAddr[0] = 0x100;
    t->confISA.isa[0].baseAddr[1] = 0x110;
    t->confISA.isa[0].codeCheck = 0x1100;
    t->confISA.isa[0].maskCheck = 0xFFFF;
    t->confISA.isa[0].maxChAdapter = 4;
    t->confISA.isa[0].offsetCheck = 4;
    //-----------------------------------------------------
    t->confISA.isa[1].typeNode = E_NODE_PV;
    t->confISA.isa[1].idNode = 0;
    t->confISA.isa[1].typeAdapter = E_A_TRREL48;
    t->confISA.isa[1].numBaseAddr = 4;
    t->confISA.isa[1].baseAddr[0] = 0x120;
    t->confISA.isa[1].baseAddr[1] = 0x130;
    t->confISA.isa[1].baseAddr[2] = 0x140;
    t->confISA.isa[1].baseAddr[3] = 0x150;
    t->confISA.isa[1].codeCheck = 0x7070;
    t->confISA.isa[1].maskCheck = 0xFFFF;
    t->confISA.isa[1].maxChAdapter = 48;
    t->confISA.isa[1].offsetCheck = 6;

    //------------------------------------------------------
    t->confISA.isa[2].typeNode = E_NODE_PV;
    t->confISA.isa[2].idNode = 0;
    t->confISA.isa[2].typeAdapter = E_A_IR;
    t->confISA.isa[2].numBaseAddr = 2;
    t->confISA.isa[2].baseAddr[0] = 0x160;
    t->confISA.isa[2].baseAddr[1] = 0x170;
    t->confISA.isa[2].codeCheck = 0xF000;
    t->confISA.isa[2].maskCheck = 0xFFF0;
    t->confISA.isa[2].maxChAdapter = 12;
    t->confISA.isa[2].offsetCheck = 4;
    
    //------------------------------------------------------
    t->confISA.isa[3].typeNode = E_NODE_PV;
    t->confISA.isa[3].idNode = 0;
    t->confISA.isa[3].typeAdapter = E_A_DAC16;
    t->confISA.isa[3].numBaseAddr = 3;
    t->confISA.isa[3].baseAddr[0] = 0x1b0;
    t->confISA.isa[3].baseAddr[1] = 0x1a0;
    t->confISA.isa[3].baseAddr[2] = 0x1c0;
    t->confISA.isa[3].codeCheck = 0x4000;
    t->confISA.isa[3].maskCheck = 0xE000;
    t->confISA.isa[3].maxChAdapter = 16;
    t->confISA.isa[3].offsetCheck = 0;
    //------------------------------------------------------
    t->confISA.isa[4].typeNode = E_NODE_PV;
    t->confISA.isa[4].idNode = 0;
    t->confISA.isa[4].typeAdapter = E_A_IP;
    t->confISA.isa[4].numBaseAddr = 1;
    t->confISA.isa[4].baseAddr[0] = 0x1d0;
    t->confISA.isa[4].codeCheck = 0xF000;
    t->confISA.isa[4].maskCheck = 0xFFF0;
    t->confISA.isa[4].maxChAdapter = 16;
    t->confISA.isa[4].offsetCheck = 4;
    //------------------------------------------------------
    t->confISA.isa[5].typeNode = E_NODE_PV;
    t->confISA.isa[5].idNode = 1;
    t->confISA.isa[5].typeAdapter = E_A_ITP;
    t->confISA.isa[5].numBaseAddr = 5;
    t->confISA.isa[5].baseAddr[0] = 0x1e0;
    t->confISA.isa[5].baseAddr[1] = 0x1f0;
    t->confISA.isa[5].baseAddr[2] = 0x200;
    t->confISA.isa[5].baseAddr[3] = 0x210;
    t->confISA.isa[5].baseAddr[4] = 0x220;
    t->confISA.isa[5].codeCheck = 0x114;
    t->confISA.isa[5].maxChAdapter = 2;
    t->confISA.isa[5].maskCheck = 0xFFFF;
    t->confISA.isa[5].offsetCheck = 0;
 

   
}
template<> void HAL::addParamToTable(uint32_t idParam,int16_t idCh,uint16_t idPackCh,uint16_t idParamCh, bool bit  )
{
    HAL* t = HAL::obj();

    uint16_t index = t->paramTable.param[idParam].num;
    t->paramTable.param[idParam].idCh[index]        = idCh;
    t->paramTable.param[idParam].idPackCh[index]    = idPackCh;
    t->paramTable.param[idParam].idParamCh[index]   = idParamCh;
    if(bit == true)
        t->paramTable.param[idParam].bit[index]     = 1;

    t->paramTable.param[idParam].num++;

    if((HAL::obj()->paramTable.num-1) < idParam)
        HAL::obj()->paramTable.num = idParam + 1;
}
template<> void HAL::addParamToTable(uint32_t idParam,int16_t idCh,uint8_t (*idPackCh)(uint8_t),uint8_t value, uint16_t idParamCh )
{
    HAL* t = HAL::obj();

    uint16_t index = t->paramTable.param[idParam].num;
    t->paramTable.param[idParam].idCh[index]        = idCh;
    t->paramTable.param[idParam].idPackCh[index]    = idPackCh(value);
    t->paramTable.param[idParam].idParamCh[index]   = idParamCh;
    t->paramTable.param[idParam].bit[index]         = 1;

    t->paramTable.param[idParam].num++;

    if((HAL::obj()->paramTable.num-1) < idParam)
        HAL::obj()->paramTable.num = idParam + 1;
}
template<> void HAL::addPackToTable(uint32_t idParam, uint8_t hBit, uint8_t lBit, uint8_t sign, float scale)
{
    packTable.packer[idParam].hBit  = hBit;
    packTable.packer[idParam].lBit  = lBit;
    packTable.packer[idParam].sign  = sign;
    packTable.packer[idParam].scale = scale;

    HAL::obj()->packTable.num++;
}
//! загрузка прошивок
template<> void HAL::downloadFirmware()
{
    std::cout << "========================================" << std::endl;
    std::cout<<std::setw(30)<<std::left<<"HAL: download firmware .... "<<std::endl;
}
template<> void HAL::updateVerConf()
{
    uploadFile(pathToConf,"ethTable.bin");
    uploadFile(pathToConf,"ethPortTable.bin");
    uploadFile(pathToConf,"nameTable.bin");
    uploadFile(pathToConf,"confISA.bin");
    uploadFile(pathToConf,"packTable.bin");
    uploadFile(pathToConf,"paramTable.bin");
    uploadFile(pathToConf,"chTable.bin");
}
template <> void HAL::init() {
    std::cout << std::endl;
    //! создание директорий для файлов модели
    createDir(pathToConf);
    //! зачитываем таблицы из файлов
    readConfFiles();
    //! зачитываем тестовые варианты таблиц
    loadTestTables();
    //! идентификация объекта
    identifySelf(nameTable,
                 typeCurrentNode,
                 currentNode,
                 currentNameNode,
                 currentGlobalTypeNode,
                 status);
    //! поднимаем сетевые  интерфейсы
    upNetIf(&ethTable,
             typeCurrentNode,
             currentNode);
    //! инициализация подсистемы статусов
    initStatusSystem();
    //! создание потока для обеспечения обслуживания
    initQ();
    //! инициализация плат PCI
    initPCI();
    //! инициализация EISA, если подключена
    initEISA();
    //! инициализация плат ISA
    initISA();
    //! создаем каналы для обмена данных
    createChAR();
    //! создаем объекты для обмена по МКИО
    createChMIL();
    std::cout << "========================================" << std::endl;
}
 void UpdateImageOS()
{
    HAL::obj()->updateVerOS(HAL::obj()->typeCurrentNode);
}

