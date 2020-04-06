#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H


#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <list>

#include "platform.h"
#include "ICalculateElement.h"

class ThreadSeparate
{
public:
    ThreadSeparate(std::string namePth,      //имя потока
                  ICalculateElement *obj,
                  int priority_=100);  //указатель на объект, который погружается в поток
      
    //! свойства потока
    void setPriority(int p)
    {
        //! Здесь нужна проверка на валидность номера приоритета
        priority_=p;
    }
    int priority(){return priority_;}
    void setUseTimer(bool value)
    {
        useTimer = value ;
    }
    //! создать поток
    virtual void create() = 0;
    //! текущий свободный сигнал
    //static int curFreeSig;
    //! имя потока
    std::string name;
    //private:
    //! приоритет потока
    int priority_;
    int policy;
    //! признак того, что поток закончил расчет
    bool finishCalc;
    //! кол-во пропущенных вызовов
    int missingCallCount;
    //! аттрибуты потока
    //pthread_attr_t attr;
    //! идентификатор потока
    int  idThread;
    //! номер сигнала
    int signal_;
    //! настрйки таймера
    long nsec; //10^-9 сек
    long sec;  // сек
    //! признак использования таймера(true)/бесконечный цикл(false)
    bool useTimer;
    //timer_t timerId;
    uint32_t idClass;
    uint32_t idObj;
    //! инкапсулированный объект
    ICalculateElement *obj;
    //! имя потока
    std::string namePth;
};

//! класс менеджер потоков
class ThreadManager
{
public:
    ThreadManager()
    {
        
    }
    //! кусок памяти для обслуживнаия МКИО
    static ThreadManager* obj();

    //! остановить все потоки
    virtual void stopAllThreads() {};
    void append(ThreadSeparate *thread)
    {
        threads.push_back(thread);
    }
   std::list<ThreadSeparate* > *listThreads(){return &threads;}
private:
   std::list<ThreadSeparate* > threads;
   static ThreadManager *th;
};

#ifdef VXWORKS_PLATFORM
    #ifdef VXWORKS_SIM
        #include "vxThreadManager.h"
        //typedef ThreadSeparate<ThreadManagerVx>        Thread;
    #else
        #include "vxThreadManager.h"
        //typedef ThreadSeparate<ThreadManagerVx>        Thread;
    #endif
#else
    #ifdef QNX_PLATFORM
        #include "qnxThreadManager.h"
    #else
        #include "./b/bThreadManager.h"
    #endif
#endif

#endif
