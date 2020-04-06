#include "threadManager.h"

ThreadManager* ThreadManager::th = 0;
ThreadManager* ThreadManager::obj() {
    if (th == 0)
        th = new ThreadManager();

    return th;
}
ThreadSeparate::ThreadSeparate(std::string name, ICalculateElement *obj_, int prior)
{
    namePth           = name;
    //! объект который будет запускаться
    obj               = obj_;
    //! идентификатор объекта
    //idObj             = obj->idObj;
    //idClass           = obj->idClass;
    //! признак окончания расчета
    finishCalc        = true;
    //! признак использования таймера
    useTimer          = false;
    //! кол-во пропущенных вызовов
    missingCallCount  = 0;
    //! настройка таймера
    nsec              = obj_->step*1000000000.;
    sec               = 0;
    //signal_            = curFreeSig;
    //! иницаиализация приоритета
    priority_         = prior;
}

