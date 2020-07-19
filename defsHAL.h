#ifndef DEFS_HAL_H
#define DEFS_HAL_H

#include "platform.h"

#define MAX_ADR_AR      132
#define MAX_WORD_AR     132
#define MAX_LEN_IP_STR  18
#define MAX_ADDR_AR_CYC 10
//! общее кол-во описанных каналов
#define MAX_CH_SYS 576
#define MAX_PARAM_SYS 2048
#define MAX_PARAM_CH 16
#define MAX_NUM_PACKER 2048
#define MAX_TABLE_NODE 9
//! максимальное число адаптеров одного типа в одном узле
#define MAX_ADAPTER_ISA 32
#define MAX_ADAPTER_TYPE_ISA 8
#define MAX_ETH_IF 64
#define MAX_ETH_PORT 24
#define NUM_MIL_TABLE 16
//! тип узла
enum TypeNodeHAL
{
    E_NODE_NONE = 0x0000, /*тип узла - неопределен*/
    E_NODE_PV   = 0x1000, /*тип узла - переферийный вычислитель*/
    E_NODE_CV   = 0x2000, /*тип узла - центральный вычислитель*/
    E_NODE_EISA = 0x3000, /*тип узла - расширитель шины EISA*/
    E_NODE_KGOO = 0x4000, /*тип узла - для системы визуализации*/
    E_NODE_LMI  = 0x5000, /*тип узла - место ЛМИ*/
    E_NODE_PKIP = 0x6000, /*тип узла - имитатор для КСУ*/
    E_NODE_PW   = 0x7000  /*тип узла - ЛМИ с управлением питанием*/
};
//! тип канала
enum TypeChHAL
{
    E_CH_NONE   = 0x0,
    E_CH_RK     = 0x1,  /* канал разовых команд*/
    E_CH_ADC    = 0x2,  /* канал АЦП*/
    E_CH_DAC    = 0x3,  /* канал ЦАП*/
    E_CH_DACWAV = 0x4,  /* канал ЦАП с семплированием*/
    E_CH_MIL    = 0x5,  /* канал МКИО*/
    E_CH_AR     = 0x6,  /* канал ДПК*/
    E_CH_SDC    = 0x7,  /* канал скоростных дискретных команд*/
    E_CH_FC_AE  = 0x8,  /* канал FC-AE*/
    E_ETH       = 0x9,  /* канал Ethernet */
    E_GEN_U     = 0xA,  /* канал 3ех фазных напряжений с регулируемым напряжением  и  частотой */
    E_IR        = 0xB,  /* канал имитатора сопротивлений */
    E_IP        = 0xC,  /* канал потенциометра */
    E_ITP       = 0xD,  /* канал термопары*/
    E_GEN_NU    = 0xE   /* канал 3ех фазных напряжений c нерегулируемым напряжением*/
};
enum TypeAdapterISA
{
    E_A_TRREL48 = 0x1, /*плата передатчик разовых команд*/
    E_A_RSREL48 = 0x2, /*плата приемник разовых команд*/
    E_A_DAC16   = 0x3, /*плата ЦАП 16 каналов*/
    E_A_DACWAV  = 0x4, /*плата ЦАП с семплированием*/
    E_A_ADC32   = 0x5, /*плата АЦП 32 канала*/
    E_A_GEN     = 0x6, /*плата 3-фазного напряжения*/
    E_A_IR      = 0x7, /*плата имитаторов сопротивления*/
    E_A_IP      = 0x8, /*плата имитаторов потенциометра*/
    E_A_ITP     = 0x9  /*плата имитаторов термопары*/

};
enum TypeChIO
{
    E_CH_IO_INPUT,
    E_CH_IO_OUTPUT
};
//#ifdef VXWORKS_PLATFORM
//#pragma pack(1)
//#else
//#pragma pack(push,1)
//#endif

//! описание
//! общее описание канала
typedef struct TSettingCh_
{
    //! номер адаптера
    int8_t  numAdapter;//-1, не используется или не задан
    //! номер канала
    uint8_t  numCh;
    //! направление передачи
    uint8_t ioCh;//io == 0 (input), io == 1 (output)
    //! готовность данных для передачи
    uint8_t readyToWrite;
    uint8_t readyToRead;
    //! признак того, что канал активен на текущем узле
    uint8_t activeLocal;
    //! блокировка по записи
    uint8_t blockWrite;
    //! блокировка по чтению
    uint8_t blockRead;
}TSettingCh;
//! описание канала РК
typedef struct TDesRK_
{
    uint8_t value;
}TDesRK;

typedef struct TDesDAC_
{
    uint16_t volt;
}TDesDAC;

typedef struct TDesIR_
{
    int16_t value; //код
    uint16_t min;
    uint16_t max;
}TDesIR;
typedef struct TDesIP_
{
    int16_t value; //код
    uint16_t max;
}TDesIP;
typedef struct TDesITP_
{
    int16_t value; //код
    uint8_t  type;   //тип термопары L = 0, K = 1
    float    max; //mv
}TDesITP;
typedef struct TDesGen_U_
{
    float u1;     //код напряжения
    float u2;     //код напряжения
    float u3;     //код напряжения
    //uint8_t phase;   //разрешение фазы(A = 0x00, B = 0x1, C = 0x2, Резерв = 0x3)
    float freq;      //
}TDesGen_U;
typedef struct TDesGen_NU_
{
    float freq;      // частота
}TDesGen_NU;
//! описание канала ДПК
typedef struct TDesAr_
{
    uint8_t numIndAddr[MAX_ADDR_AR_CYC];  //кол-во индексов адресов
    uint8_t freq;                       //частота передачи данных
    uint8_t rev;                        //ревизия передачи данных РТМ изм.3/изм.2
    uint8_t type;                       //тип передачи данных(циклический/ациклический)
    uint8_t indAddr[MAX_ADR_AR][MAX_ADDR_AR_CYC];   //индексы адресов из массива адресов
    uint8_t curIndArray;                // текущий индекс массива
    float freqArray;                    // частота между массивами
    uint8_t numArray;                   // кол-во массивов, если кол-во массивов = 0, то массивы не нужно индексировать
    uint8_t numAddr;                    // кол-во адресов в массиве адресов
    uint8_t addr[MAX_ADR_AR];           // массив адресов
    //! признак активного канала( 0 - канал выключен/ 1 - канал включен)
    uint8_t active;
    uint32_t value[MAX_WORD_AR];        //значение по каждому канальному адресу
    uint8_t write;                      //признак записи параметра   
}TDesAr;

typedef struct TDesEth_
{
    //! порт UDP
    uint16_t port;
    char ip[MAX_LEN_IP_STR];
}TDesEth;
//! описание канала МКИО
typedef struct TDesMIL_
{   
    //! тип передачи
    uint8_t typeTrans; //КоУ(0), ОуК(1), Мон(2)
    uint8_t addr;      //адрес ОУ
    //uint8_t numAddr;
    uint8_t subAddr;   //подадрес ОУ
    uint8_t numWord;   //кол-во слов для передачи
    //! признак активного канала( 0 - канал выключен/ 1 - канал включен)
    uint8_t active;
    union{
    uint16_t ui16[32];//значение по каждому слову
    uint32_t ui32[16];
    }s;
    // признаки записи
    union{
        uint8_t write16[32];
        uint8_t write32[16];
    }w;
}TDesMIL;
//! описание канала
typedef struct TCh_
{
    //! имя канала
    char nameCh[80];
    //! идентифкатор канала
    uint32_t idCh;
    //! тип узла
    TypeNodeHAL typeNode;
    //! идентификатор узла
    uint8_t idNode;
    //! тип канала
    TypeChHAL typeCh;
    //! настройки канала
    TSettingCh setting;
    //! описание доп. данных
    uint8_t desData[sizeof(TDesAr)];
}TCh;

//! таблица с описанием параметров
typedef struct TTableChHAL_
{
    //! кол-во каналов
    uint16_t numCh;
    //! описание каналов
    TCh ch[MAX_CH_SYS];
}TTableChHAL;

typedef struct TTablePackParam_
{
    uint8_t      hBit;     /* старший бит */
    uint8_t      lBit;     /* младший бит*/
    uint8_t      sign;     /* знак*/
    float        scale;    /* масштаб */
}TTablePackParam;
typedef struct TTablePack_
{
    uint16_t num;
    TTablePackParam packer[MAX_NUM_PACKER];
}TTablePack;
//! описание параметра
typedef struct TParamHAL_
{
    uint8_t     num;
    int16_t     idCh[32];     /* идентификатор каналов*/
    uint16_t    idParamCh[32];/* идентификатор параметра*/
    uint16_t    idPackCh [32]; /* прямой индекс в таблицу упаковок*/
    uint8_t     bit[32];
    uint32_t    value;       /* адрес на буффер со значением*/
    float       initValue;   /* значение по умолчанию*/
}TParamHAL;
typedef struct TDesNode
{
    //! тип узла
    TypeNodeHAL typeNode;
    //! идентификатор узла
    uint8_t idNode;
    //! название модуля
    char name[80];
    //! MAC адрес интерфейса
    char mac[32];
}TNameNode;
typedef struct TTableNode_
{
    uint8_t numNode;
    TNameNode node[MAX_TABLE_NODE];

}TTableNode;
//! таблица с описанием параметров
typedef struct TTableParam
{
    //! кол-во параметров в таблице
    uint32_t num;
    //! список параметров
    TParamHAL param[MAX_PARAM_SYS];//индекс это шлобальный idParam
}TTableParam;
typedef struct TSettingNode_
{
    uint32_t idNode;
    //! уникальный массив для узла для преобразования индексов
    uint32_t *convRkIn;
    uint32_t *convRkOut;
    uint32_t *convArIn;
    uint32_t *convArOut;

    //! описание каналов РК
    TSettingCh *chRkIn;
    uint16_t lenRkIn;
    TSettingCh *chRkOut;
    uint16_t lenRkOut;
    //! описание каналов ARINC
    TSettingCh *chArIn;
    uint16_t lenArIn;
    TSettingCh *chArOut;
    uint16_t lenArOut;
    //! описание каналов МКИО
    TSettingCh *mil;
    uint8_t lenMil;

}TSettingNode;

typedef struct TDataISA_
{
    union
    {
      uint16_t u16[8];
      uint32_t u32[4];
    }w;
}TDataISA;

//! описание одного адаптера ISA
typedef struct TAdapterISA_
{
    //! тип узла
    TypeNodeHAL typeNode;
    //! идентификатор узла
    uint8_t idNode;
    //! тип адаптера
    TypeAdapterISA typeAdapter;
    //! кол-во базовых адресов
    uint8_t numBaseAddr;
    //! список базовых адресов
    uint16_t baseAddr[MAX_ADAPTER_TYPE_ISA];
    //! смещение до регистра с кодом
    uint8_t offsetCheck;
    //! код исправности
    uint16_t codeCheck;
    //! маска для выделения исправности(если требуется)
    uint16_t maskCheck;
    //! максимальное кол-во каналов на одном адаптере
    uint8_t maxChAdapter;
    //! текущее состояние
    //TDataISA data[MAX_ADAPTER_TYPE_ISA];
}TAdapterISA;

//! конфигурация адаптеров на шине ISA
typedef struct TConfISA_
{
  uint16_t numAdapters;
  TAdapterISA isa[MAX_ADAPTER_ISA];
}TConfISA;

typedef struct TEthIf_
{
    //! тип узла
    TypeNodeHAL fromTypeNode;
    //! идентификатор узла
    uint8_t fromIdNode;
    //! номер интерфейса
    int8_t numIf;
    //! обмен с кааим узлом
    TypeNodeHAL toTypeNode;
    uint8_t toIdNode;
//    //! порт для обмена данными между узлами
//    uint16_t portJNode;
    //! ip - адрес интерфейса, который нужно поднять
    char ip[16];
    uint32_t ip_int;
    //! имя интерфейса
    char nameIf[8];
    //! MAC адрес интерфейса
    char mac[32];
    //! признак наличия соединения
    //! 0 - узел не доступен
    //! 1 - узел доступен, но не отвечает
    //! 2 - узел доступен и отвечает(подсистема HAL запустилась)
    uint8_t isConnect;
}TEthIf;


//! таблица с назначением ethernet каналов на вычислителях
typedef struct TTableEth_
{
    int numEth;
    TEthIf eth[MAX_ETH_IF];
}TTableEth;

typedef struct TTableEthPort_
{

    uint8_t numPort;
    uint16_t port[MAX_ETH_PORT];
}TTableEthPort;

typedef struct TTableMIL_
{
    //! тип узла
    TypeNodeHAL typeNode;
    //! идентификатор узла
    uint8_t idNode;
    //! кол-во адаптеров в режиме  ОУ
    uint8_t numAdapterRegOU;
    //! кол-во адаптеров в режиме  Монитора
    uint8_t numAdapterRegMon;
    //! кол-во адаптеров в режиме  КОУ
    uint8_t numAdapterRegKOU;
}TTableMIL;

typedef struct TConfPCI_
{
    //! кол-во элементов
    uint8_t numMil;
    TTableMIL mil[NUM_MIL_TABLE];
}TConfPCI;
typedef struct TDesFromEISA_
{
    uint16_t indexCh;
    uint8_t  value[32];
}TDesFromEISA;
typedef struct TDesToEISA_
{
    TypeAdapterISA  type;
    uint16_t        idCh;
    uint8_t         io;
    uint16_t        addr;
    uint8_t         value[32];
    uint16_t        size;
}TDesToEISA;
typedef struct TSetupHAL_
{
    //!  обновление версии
    uint8_t updateVer;
    //! признак обновления bootrom
    uint8_t updateBootrom;
    //! признак обновления файлов с конфигурацией системы
    uint8_t downloadConf;
    //! обновление файлов для адаптеров
    uint8_t downloadFirmware;
    //! быстрое обновление версии(с остановкой модели)
    uint8_t updateQuickVer;        
}TSetupHAL;
typedef struct TStatusHAL_
{
    //!  текущий номер обновленной версии
    uint8_t updateVer;
    //! номер версии операционной системы
    char    verOs[15];
    //! номер версии модели
    char    verModel[16];
}TStatusHAL;

typedef struct TPacketStatus_
{
    //! тип узла
    TypeNodeHAL typeNode;
    //! идентификатор узла
    uint8_t idNode;
    //! статус
    uint8_t status;
}TPacketStatus;


#endif // HAL_H
