#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include "mppm.h"
#include "connectToMPPM.h"
#include "chdev.h"
#include "hal.h"

AddReqMPPM::AddReqMPPM(uint32_t idClass, ICalculateElement *mppm_):ICalculateElement(idClass)
{
    //! указываем зависимости
    mppm  = mppm_;
    prAdd = false;
    msgQueue.msgCreate("/mppm",256,sizeof(TRecordMPPM),MsgQueue::E_MSG_RDWR);
    msgQueue.msgSetAttr(0); //block
    setFreq(ICalculateElement::HzInf);
    setStart();
}
void AddReqMPPM::calculate()
{
    TRecordMPPM packet;
    memset((void*)&packet,0,sizeof(packet));

    //! ожидаю данных от MPPM
    //if(msgQReceive(VxToMPPM::obj()->msgToMPPM, (char*)&packet, sizeof(packet), WAIT_FOREVER) == ERROR)
    /*if(msgQReceive(VxToMPPM::obj()->msgToMPPM, (char*)&packet, sizeof(packet), WAIT_FOREVER) == ERROR)
        std::cout<<"AddReqMPPM: error in msgQReceive"<<std::endl;*/

    bool dataReady = msgQueue.msgReceive((uintptr_t)&packet, sizeof(packet));
    if(dataReady == false)
    	return;
    
    prAdd = true;
    MPPM *mppm_el = static_cast<MPPM * > (mppm);
    
    TTableMPPM tab;
    tab.addr = packet.addr;
    tab.name = std::string(packet.name);
    tab.size = packet.size;
    //! поиск такого уже добавленого модуля
    
    std::vector<TTableMPPM>::iterator it;
    for(it = mppm_el->table.begin();it!= mppm_el->table.end();++it)
    {
        if(it->name == tab.name)
        {
            prAdd = false;
            *it = tab;
            //std::cout<< "Warning: AddReqMPPM: Module = " <<packet.name<<" already added."<<std::endl;
            break;
        }
    
}
    if(prAdd == true)
    	mppm_el->table.push_back(tab);
}
CyclicServMPPM::CyclicServMPPM(uint32_t idClass, ICalculateElement *mppm_):ICalculateElement(idClass)
{
    //! указываем зависимости
    mppm = mppm_;
    setFreq(ICalculateElement::Hz6_25);
    setStart();
}
void CyclicServMPPM::calculate()
{
}
MPPM::MPPM(uint32_t idClass):ICalculateElement(idClass)
{
    udpSocket  = new UdpSocket;
    udpSocket->init(HAL::obj()->ethPort(ETH_PORT_REC_MPPM),"");
    int res = udpSocket->bindTo();
    if(res == ERROR)
    {
        std::cout<<"MPPM: Can`t bind socket"<<std::endl;
    }
    //! создаем поток для добавления запросов
    std::string strAddReq = "pthAddReqMPPM";
    pthrAddReq = new Thread(strAddReq,new AddReqMPPM(ID_ADD_REQ_MPPM, this), 55);
    pthrAddReq->setUseTimer(false);
    pthrAddReq->create();

    //! создаем поток для обработки циклических запросов
    std::string strCyclServ = "pthCyclServMPPM";
    pthCyclServ = new Thread(strCyclServ,new CyclicServMPPM(ID_CYD_SERV_MPPM, this),55);
    pthCyclServ->setUseTimer(true);
    pthCyclServ->create();

    //! указываем зависимости
    setFreq(ICalculateElement::Hz6_25);
    setStart();
}
void MPPM::init()
{
    ICalculateElement::init();
}
void MPPM::calculate()
{
    if(udpSocket<0)
        return;
    
    getControlCommand();
}
//! получить команду от ui интерфейса
void MPPM::getControlCommand()
{
    //! получение данных
    int status = reciveDataHead();
    if(status == sizeof(THeadRequest))
    {

        switch(headReq.type)
        {
        case R_T_LISTPM:
        {
            if(headReq.rwm == E_REQ_READ)
            {
                status = readRequestMem();
            }else
                printf("\nWarning mppm-thread: \n");

            if(status == OK)
                status = sendData();
            if(status == ERROR)
                printf("MPPM: Error in func() = sendData(),%d, control.cpp\n",__LINE__);
            break;
        }
        case R_T_MODULE:
        {
            if(headReq.rwm == E_REQ_READ)
            {
                status = readRequestModule();
                if(status == ERROR)
                    printf("MPPM: Error in func() = readRequest(), control.cpp\n");
            }
            else if(headReq.rwm == E_REQ_WRITE)
            {
                status = writeRequestModule();
                if(status == ERROR)
                    printf("MPPM: Error in func() = writeRequest(), control.cpp\n");
            }

            if(status == OK)
                status = sendData();
            if(status == ERROR)
                printf("MPPM: Error in func() = sendData(),%d, control.cpp\n",__LINE__);
            break;
        }
        case R_T_SELECTED:
        {
            if(headReq.rwm == E_REQ_READ)
                status = readRequestSel();
            else if(headReq.rwm == E_REQ_WRITE)
                status = writeRequestSel();

            if(status == OK) status = sendData();
            if(status == ERROR)
                printf("MPPM: Error in func()=sendData(),%d, control.cpp\n",__LINE__);
            break;
        }
        default:
        {
            printf("\nMPPM: headReq.rwm is incorrect\n");
        }
        };
    }else //! нужно опустошить буфер
    {
        flushAllBuffer();
    }
}
int MPPM::readRequestMem()
{
    int bytes = 0;
    
    memset((void*)&memReq,0,headReq.size);

    bytes = udpSocket->reciveFrom((uint8_t*)&memReq, headReq.size);
    
    if(bytes < 0)
    {
        perror("MPPM : func = readRequestMem(), recvfrom");
        return ERROR;
    }

    if(bytes != headReq.size)
        return ERROR;

    std::string strReq((char*)memReq.name);
    for(int i = 0;i<table.size();i++)
    {
        if(table[i].name == strReq)
        {
            /*if(TableAssocieted[i].SizeType>SIZE_BUFFER)
            {
                memReq.err = SMALL_BUFFER;
                return -1;
            }*/

            /*if(TableAssocieted[i].SizeType!=request.size)
            {
                request.err = MISMATCH_SIZE_BUFFER;
                return 0;
            }*/
            /*if(TableAssocieted[i].Address<=0)
            {
                memReq.err = NULL_PTR_IN_TABLE;
                return -1;
            }
            if(TableAssocieted[i].SizeType<=0)
            {
                memReq.err = SIZE_TYPE_IS_ZERO;
                return -1;
            }*/
            //memcpy((void*)&memReq.buffer[0],(void*)TableAssocieted[i].Address,TableAssocieted[i].SizeType);

            memReq.index = i;
            //memReq.size = TableAssocieted[i].SizeType;
            return OK;
        }
    }
    return OK;
}

//! обработка запроса на чтение
int MPPM::readRequestModule(void)
{
    int i = 0;
    int bytes = 0;
    int sizeBuf = headReq.size;//sizeof(TModuleRequest);
//
    if(headReq.size>sizeof(TModuleRequest))
        sizeBuf = sizeof(TModuleRequest);

    memset((void*)&moduleReq,0,sizeof(TModuleRequest));

    bytes = udpSocket->reciveFrom((uint8_t*)&moduleReq, sizeBuf);

    if(bytes<0)
    {
        perror("MPPM : func = readRequestModule(), recvfrom, ");
        std::cout<<"headReq.size = "<<headReq.size<<std::endl;
        return ERROR;
    }

    if(bytes != headReq.size)
        return ERROR;

    
    if(moduleReq.index!=-1 && table.empty() == false)
    {
        i = moduleReq.index;
        
        /*if(table[i].size>SIZE_BUFFER)
        {
            moduleReq.err = SMALL_BUFFER;
            return -1;
        }*/

        /*if(TableAssocieted[i].SizeType!=request.size)
        {
            request.err = MISMATCH_SIZE_BUFFER;
            return 0;
        }*/
        if(i >= table.size())
            return -1;
        if(table[i].addr == 0)
        {
            moduleReq.err = NULL_PTR_IN_TABLE;
            return -1;
        }
        if(table[i].size == 0)
        {
            moduleReq.err = SIZE_TYPE_IS_ZERO;
            return -1;
        }

        int maxCount = (table[i].size/SIZE_BUFFER) + 1;    
        for(int j = 0; j<maxCount;j++)
        {
           int size = table[i].size - (j * SIZE_BUFFER);
           if(size>SIZE_BUFFER)
               size = SIZE_BUFFER;
           memcpy((void*)&moduleReq.buffer[0],(void*)(table[i].addr + j * SIZE_BUFFER),size);
           moduleReq.offset        = j * SIZE_BUFFER;
           moduleReq.sizeBuf       = size;
           moduleReq.head.size     = sizeof(TModuleRequest)-(SIZE_BUFFER - size);
           moduleReq.err           = NOT_ERROR;
           headReq.size            = moduleReq.head.size;
        

           sendData();
        }   
        
        //memcpy((void*)&moduleReq.buffer[0],(void*)table[i].addr,table[i].size);

        return OK;
    }
    moduleReq.err = NOT_FOUND_PM;
    return OK;
}
int MPPM::writeRequestModule(void)
{
    int i = 0;
    int bytes = 0;    
    int sizeBuf = sizeof(TModuleRequest);

    if(headReq.size<sizeBuf)
        sizeBuf = headReq.size;

    memset((void*)&moduleReq,0,sizeof(TModuleRequest));

    bytes = udpSocket->reciveFrom((uint8_t*)&moduleReq, sizeBuf);

    if(bytes<0)
    {
        perror("MPPM : func = writeRequestModule(), recvfrom");
        return ERROR;
    }

    if(bytes != headReq.size)
        return ERROR;
    
    if(moduleReq.index != -1)
    {
        i = moduleReq.index;

        if(table.size() < i)
            return ERROR;
        if(table[i].size > SIZE_BUFFER)
        {
           moduleReq.err = SMALL_BUFFER;
           return OK;
        }
        if(table[i].size!=moduleReq.sizeBuf)
        {
           moduleReq.err = MISMATCH_SIZE_BUFFER;
           return OK;
        }
        if(table[i].addr == 0)
        {
           moduleReq.err = NULL_PTR_IN_TABLE;
           return OK;
        }
        if(table[i].size == 0)
        {
           moduleReq.err = SIZE_TYPE_IS_ZERO;
           return OK;
        }      

        memcpy((void*)table[i].addr,(void*)(&moduleReq.buffer[0] + moduleReq.offset),table[i].size);
        moduleReq.err = NOT_ERROR;
        return OK;
    }
    moduleReq.err = NOT_FOUND_PM;
    return OK;
}
int MPPM::reciveDataHead()
{
    int bytes = 0;

    memset((void*)&headReq,0,sizeof(THeadRequest));
    bytes = udpSocket->reciveFrom((uint8_t*)&headReq, sizeof(THeadRequest),MSG_WAITALL | MSG_PEEK);

    if(bytes < 0)
    {
        perror("MPPM : func = reciveDataHead(), recvfrom");
        return ERROR;
    }
    //RefreshUIPtr();
    return bytes;
}
void MPPM::flushAllBuffer()
{
    int bytes = ERROR;    
    do
    {
        bytes = udpSocket->reciveFrom((uint8_t*)&headReq, sizeof(THeadRequest),MSG_WAITALL);
    } while(bytes != ERROR);
        
}
int MPPM::sendData()
{
    int bytes = 0;
    uint8_t *ptrBuf = 0;
    switch(headReq.type)
    {
    case R_T_LISTPM:
    {
        ptrBuf = (uint8_t*) &memReq;
        break;
    }
    case R_T_MODULE:
    {
        ptrBuf = (uint8_t*) &moduleReq;
        break;
    }
    case R_T_SELECTED:
    {
        ptrBuf = (uint8_t*) &selectReq;
        break;
    }
    };
    if(ptrBuf != 0)
    {
        bytes = udpSocket->sendTo(ptrBuf,headReq.size, headReq.port);
        if(bytes < 0)
        {
            perror("MPPM : func = sendData()");
            return ERROR;
        }
        return OK;
    }
    return ERROR;
}
int MPPM::readRequestSel()
{  
    int bytes = 0;
  
    memset((void*)&selectReq,0,headReq.size);

    bytes = udpSocket->reciveFrom((uint8_t*)&selectReq, headReq.size);

    if(bytes<0)
    {
        perror("MPPM : func = readRequestSel(), recvfrom");
        return ERROR;
    }
    if(bytes != headReq.size)
        return ERROR;

    for(int i = 0;i<selectReq.numValues;i++)
    {
        int index = selectReq.values[i].indexInTable;
//        if(index>Length_Table)
//        {
//            selectReq.values[i].err=INDEX_TOO_MACH;
//            return OK;
//        }
        if(index>table.size())
            return OK;
        if(index >= 0)
        {
            if(selectReq.values[i].offset>(table[i].size-1))
            {
                selectReq.values[i].err = OFFSET_TOO_MACH;
            }

           memcpy((void*)(selectReq.values[i].value),
                   (void*)(table[index].addr + selectReq.values[i].offset),
                   selectReq.values[i].byte);
        }else
        {
            selectReq.values[i].err = INDEX_IS_NEG;
            return OK;
        }
    }
    return OK;
}
int MPPM::writeRequestSel()
{
    int bytes = 0;

    int sizeBuf = sizeof(selectReq);
    
    if(headReq.size<sizeBuf)
        sizeBuf = headReq.size;

    memset((void*)&selectReq,0,headReq.size);
    bytes = udpSocket->reciveFrom((uint8_t*)&selectReq, sizeBuf);

    if(bytes < 0)
    {
        perror("MPPM : func = writeRequestSel(), recvfrom");
        return ERROR;
    }
    if(bytes != headReq.size)
        return ERROR;

    for(int i = 0;i<selectReq.numValues;i++)
    {
        int index = selectReq.values[i].indexInTable;
        if(i >table.size())
            return OK;
/*        if(index>=Length_Table)
        {
            selectReq.values[i].err=INDEX_TOO_MACH;
            return OK;
        }*/
        if(index >= 0)
        {
            if(selectReq.values[i].offset>(table[i].size - 1))
            {
                selectReq.values[i].err = OFFSET_TOO_MACH;

            }
            memcpy((void*)(table[index].addr + selectReq.values[i].offset),
                   (void*)(selectReq.values[i].value),
                   selectReq.values[i].byte);
/*            memcpy((void*)(selectReq.values[i].value),
                   (void*)(TableAssocieted[index].Address+selectReq.values[i].offset),
                   selectReq.values[i].byte);*/
        }else
        {
            selectReq.values[i].err = INDEX_IS_NEG;
            return OK;
        }
    }
    return OK;
}
