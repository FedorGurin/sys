#ifndef MPPM_H
#define MPPM_H

#include "ICalculateElement.h"
#include "templateUdpSocket.h"
#include "templateMsgQueue.h"
#include "threadManager.h"

//! Размер буффера для передачи/получения
#define SIZE_BUFFER 12048
//! Длина массива с именем программного модуля
#define SIZE_NAME 80
//! длина массива с IP адресом
#define SIZE_NAME_IP 16
//! кол-во параметров в одном пакете TSelectedRequest_
#define SIZE_VALUES 64
//! перечень ошибок
enum REQ_ERROR{
    NOT_ERROR            = 0, /*Ошибок нет*/
    MISMATCH_SIZE_BUFFER = 1, /*Не соотвествие размеров буферов между передающими и принимающими программами */
    SMALL_BUFFER         = 2, /*Маленький размер буферов*/
    NOT_FOUND_PM         = 3, /*Имя программных модулей не совпадает*/
    NULL_PTR_IN_TABLE    = 4, /*Нулевой указатель в таблице программных модулей*/
    SIZE_TYPE_IS_ZERO    = 5, /*Нулевой размер типа в таблице программных модулей*/
    INDEX_IS_NEG         = 6, /*Индекс имеет орицательное значение*/
    INDEX_TOO_MACH       = 7, /*Значение индекса превышает размер таблицы запросов*/
    OFFSET_TOO_MACH      = 8, /*смещение слишком большое*/
    FLUSH_INDEX          = 9  /*сбросить индексы*/
};
enum REQ_RWM{
    E_REQ_NO               = -1, // запрос отсутствует
    E_REQ_READ             = 0,  // запрос на запись
    E_REQ_WRITE            = 1,  // запрос на чтение
    E_REQ_MASK             = 2,  // запрос маски
    E_REQ_CYCLIC           = 3,  // запрос на добавление циклического запроса
    REM_CYCLIC_REQ         = 4
};

enum REQ_TYPE{
    R_T_MODULE               = 0,
    R_T_SELECTED             = 1,
    R_T_LISTPM               = 2
};
//#pragma pack(push,1)
//! Структура запроса для отправки/получения
typedef struct THeadRequest_
{
    //! идентификатор пакета (для идентификации одинаковых пакетов)
    uint32_t id;
    //! идентификатор пакета (для обработки запроса)
    uint32_t uid;
    //! Назначение сообщения:
    //              0 - чтение,
    //              1 - запись,
    //              2 - маска
    uint8_t rwm;
    //! Тип сообщения
    //              0 - запрос адреса ПМ
    //              1 - запрос для работы со всей структурой
    //              2 - запрос с выбранными параметрами(из разных структур)
    uint8_t type;
    //! ip адрес(отправителя)
    uint8_t ip[SIZE_NAME_IP];//! например, 127.0.0.1
    //! порт(приемника отправителя)
    uint16_t port;
    //! уникальный индекс узла приема
    int8_t indexNode;
    //! размер пакета, байты
    uint32_t size;
}THeadRequest;
//! структура для работы сразу со всем модулем
typedef struct TModuleRequest_
{
    //! заголовок
    THeadRequest head;
    //! код ошибки
    uint8_t err;
    //! идентификатор запроса(элемент массива в таблице запроса)
    int16_t index;
    //! смещение внутри структуры
    uint32_t offset;
    //! размер структуры в буффере
    uint32_t sizeBuf;
    //! буффер с данными
    uint8_t buffer[SIZE_BUFFER];
}TModuleRequest;
//! структура для запроса адреса по строковому идентификатору
typedef struct TMemRequest_
{
    //! заголовок пакета
    THeadRequest head;
    //! уникальное имя участка памяти
    uint8_t name[SIZE_NAME];
    //! возвращаемое значение
    uint32_t addr;  //прямой адрес в памяти структуры
    uint32_t size;  //размер структуры
    int16_t index; //индекс в таблице
}TMemRequest;

//! Для работы с ограниченным набором параметров параметров
typedef struct TValueRequest_
{
    uint8_t err;               //! код ошибки
    uint8_t byte;              //! кол-во байт значения
    int16_t indexInTable;   //! индекс в таблице с начальным адресом
    uint32_t offset;         //! смещение
    //unsigned addr;          //! прямой адрес в памяти
    uint8_t value[8];          //! значение(размер занимаемой памяти зависит от byte)
}TValueRequest;

typedef struct TSelectedRequest_
{
    //! заголовок пакета
    THeadRequest head;
    //! кол-во параметров
    uint16_t numValues;
    //! список значений
    TValueRequest values[SIZE_VALUES];
}TSelectedRequest;

typedef struct TTableMPPM_
{
    //! имя записи
    std::string name;
    //! идентификатор
    uint16_t id;
    //! адрес структуры
    uintptr_t addr;
    //! размер структуры
    uint32_t  size;
}TTableMPPM;

typedef struct TSettingMPPM_
{
    uint16_t portRecive;   
}TSettingMPPM;

class AddReqMPPM:public ICalculateElement
{
public:
    AddReqMPPM(uint32_t idClass, ICalculateElement *mppm);
    
    virtual void calculate();
    virtual void finite(){}   
    
    bool prAdd; //команда на добавление нового модуля
    //! connectToMsg
    MsgQueue msgQueue;
    ICalculateElement *mppm;
};
class CyclicServMPPM:public ICalculateElement
{
public:
    CyclicServMPPM(uint32_t idClass, ICalculateElement *mppm);

    virtual void calculate();
    virtual void finite(){}

    ICalculateElement *mppm;
};
//! класс для взаимодействия с модулем MPPM
class  MPPM:public ICalculateElement
{
public:
    MPPM(uint32_t idClass);

    //! обобщенный интерфейс
    virtual void init();
    virtual void calculate();
    virtual void finite(){}

    int readRequestModule(void);
    int writeRequestModule(void);
    int reciveDataHead();
    int sendData();
    int readRequestSel();
    int writeRequestSel();
    int readRequestMem();
    void getControlCommand();
    //! сбросить весь приемный буфер
    void flushAllBuffer();

    //! структуры для обработки запросов
    //! заголовок
    THeadRequest        headReq;
    //! запрос на получение всей структуры
    TModuleRequest      moduleReq;
    //! запрос на получение списка запросов
    TSelectedRequest    selectReq;
    //! запрос на получение участка памяти
    TMemRequest         memReq;
    //! массив с запомненными циклическими запросами
    std::vector<TSelectedRequest> selCyclic;
    std::vector<TModuleRequest>   moduleCyclic;
    //! таблица с зарегистрироваными структурами для обмена
    std::vector<TTableMPPM> table;    
//private:
    //! указатели
    UdpSocket *udpSocket;
    ThreadSeparate *pthrAddReq;
    ThreadSeparate *pthCyclServ;
    std::vector<uint16_t> indexes;
};

#endif
