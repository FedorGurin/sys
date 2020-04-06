#ifndef GLHAL_HPP
#define GLHAL_HPP
#include "hal.h"
#include "layerRK.h"
#include "layerMIL.h"
#include "bits.h"
#include "layerSpecRTM.h"
#include "math_func.h"
//#include "layerSpecMIL.h"

enum IO_COMMAND_CH
{
    IO_SWITCH_ON_OFF        , //вкл(выкл) канала
    IO_READ_FROM_ADAPTER    , //принудительное чтение из адаптера
    IO_WRITE_TO_ADAPTER     , //принудительная запись в адаптер
    IO_SEND_DATA            , //выдача данных
    IO_CHECK_KC_MIL           //проверка прихода КС по МКИО
};


//! преобразование чисел в/из кода Arinc/MIL во внутренние параметры
template <typename B,typename Type> void convParam(
        uint16_t     idCh,
        uint16_t     guidParam,       /* идентификатор параметра*/
        Type&        value)
{
   
    bool isRead = B::isRead(idCh);
    TParamHAL *param = HAL::obj()->findParam(guidParam);
    TTablePackParam *pack = HAL::obj()->findPack(idCh,param);
    if(pack != 0)
    {
        uint8_t dByte = (pack->hBit - pack->lBit)>>3;
        if(pack->scale<=0.0f)
        {
           if(isRead)
           {
               if(dByte <= sizeof(*(B::getPtrToValue(idCh,guidParam))))
               {
               readCode0(B::getPtrToValue(idCh,guidParam),
                         value, /*куда прочитать*/
                         pack->hBit,
                         pack->lBit);
               }else
               {
                   uint16_t *ptr = (uint16_t*)B::getPtrToValue(idCh,guidParam);
                   uint16_t val[2];
                   val[0] = ptr[1];
                   val[1] = ptr[0];

                   readCode0((uint32_t*)val,
                           value,           /*куда прочитать*/
                           pack->hBit,      /*старший бит(hight bit)*/
                           pack->lBit);     /*младший бит(low bit)*/

               }
           }else
           {
               if(dByte <= sizeof(*(B::getPtrToValue(idCh,guidParam))))
               {
                   writeCode(B::getPtrToValue(idCh,guidParam),
                             value, /*куда записать*/
                             pack->hBit,
                             pack->lBit);
               }else
               {
                   uint16_t *ptr = (uint16_t*)B::getPtrToValue(idCh,guidParam);

                   uint16_t val[2];
                   val[0] = ptr[1];
                   val[1] = ptr[0];

                   writeCode((uint32_t*)val,
                           value,           /*куда прочитать*/
                           pack->hBit,      /*старший бит(hight bit)*/
                           pack->lBit);     /*младший бит(low bit)*/


                   ptr[0] = val[1];
                   ptr[1] = val[0];
               }
           }

        }else
        {
            if(isRead)
            {
                if(dByte<=sizeof(*(B::getPtrToValue(idCh,guidParam))))
                {
                    readCodeScale(B::getPtrToValue(idCh,guidParam),
                            value,    /*куда прочитать*/
                            pack->hBit,    /*старший бит(hight bit)*/
                            pack->lBit,     /*младший бит(low bit)*/
                            pack->sign,     /*знак*/
                            pack->scale);   /*масштаб*/
                }else
                {
                    uint16_t *ptr = (uint16_t*)B::getPtrToValue(idCh,guidParam);
                    uint16_t val[2];
                    val[0] = ptr[1];
                    val[1] = ptr[0];
                    
                    readCodeScale((uint32_t*)val,
                            value,    /*куда прочитать*/
                            pack->hBit,    /*старший бит(hight bit)*/
                            pack->lBit,     /*младший бит(low bit)*/
                            pack->sign,     /*знак*/
                            pack->scale);   /*масштаб*/
                }
            }else
            {
                if(dByte<=sizeof(*(B::getPtrToValue(idCh,guidParam))))
                {
                    writeCodeScale1(B::getPtrToValue(idCh,guidParam),
                            value,    /*куда прочитать*/
                            pack->hBit,    /*старший бит(hight bit)*/
                            pack->lBit,     /*младший бит(low bit)*/
                            pack->sign,     /*знак*/
                            pack->scale);   /*масштаб*/
                }else
                {
                    uint16_t *ptr = (uint16_t*)B::getPtrToValue(idCh,guidParam);
                    
                    uint16_t val[2];
                    val[0] = ptr[1];
                    val[1] = ptr[0];
                                        
                    writeCodeScale1((uint32_t*)val,
                            value,    /*куда прочитать*/
                            pack->hBit,    /*старший бит(hight bit)*/
                            pack->lBit,     /*младший бит(low bit)*/
                            pack->sign,     /*знак*/
                            pack->scale);   /*масштаб*/
                    
                    ptr[0] = val[1];
                    ptr[1] = val[0];
                }
        }
    }
}
}


//! чтение данных из канала
template <typename T> bool readHAL(uint32_t idCh,    //! идентификатор канала
                                   uint32_t idParam, //! идентификатор параметра
                                   T &value)
{
    TCh *ch = &(HAL::obj()->chTable.ch[idCh]);

    if(ch == 0 )
        return false;

    switch(ch->typeCh)
    {
        case E_CH_RK:
        {
            TDesRK *desRk = (TDesRK*)ch->desData;
            value = desRk->value;
            return true;
        }
        case E_CH_ADC:
        {
            break;
        }
        case E_CH_DAC:
        {
            break;
        }
        case E_CH_DACWAV:
        {
            break;
        }
        case E_CH_MIL:
        {

            break;
        }
        case E_CH_AR:
        {
            int index = 0;
            TDesAr *desAr    = (TDesAr*)ch->desData;
            TParamHAL *param = HAL::obj()->findParam(idParam);
            for(int i = 0;i<param->num;i++)
            {
                if(param->idCh[i] == idCh)
                {
                    index = i;
                }
            }
            //for(int j = 0; j< desAr->numArray; j++)
            //{
                for(int i = 0; i< desAr->numAddr; i++)
                {
                    if(desAr->addr[i] == param->idParamCh[index] )
                    {
                        value = desAr->value[i];
                        return true;
                    }
                }
            //}


            break;
        }

        case E_IR:
        {

            break;
        }
        case E_IP:
        {

            break;
        }
        case E_ITP:
        {

            break;
        }
        default:{}
    };
    return false;


}
//! запись данных из канала
template <typename T> bool writeHAL(int32_t idCh,    //! идентификатор канала
                                    uint32_t idParam, //! идентификатор параметра
                                    T &value)
{

}

//! прочитать параметр из таблицы
template <typename T> bool doHAL(int32_t idCh,    //! идентификатор канала
                                 uint32_t idParam, //! идентификатор параметра
                                 T &value)
{    
    
    if(idCh == -1)
        return false;
    TCh *ch = &(HAL::obj()->chTable.ch[idCh]);

    if(ch == 0 )
        return false;

    //! данные готовы для чтения или для записи(нужно сравнивать с предыдущим занчением)
    ch->setting.readyToWrite = 1;
//    if(ch->typeNode != HAL::obj()->typeCurrentNode ||
//       ch->idNode   != HAL::obj()->currentNode )
//    {
//        //! тогда помещаем в буфер для передачи по сети
        
//        return false;
//    }
    switch(ch->typeCh)
    {
        case E_CH_RK:
        {
            uint8_t res = value;
            //! сохранить данные в канале
            TDesRK * rk = (TDesRK*)ch->desData;
            //! еще бы копировал бы в HAL TDesRK
            
            if(ch->typeNode == HAL::obj()->typeCurrentNode &&
               ch->idNode   == HAL::obj()->currentNode )
           {
           if(ch->setting.ioCh == 1)
           {
              LayerRK::set(ch->setting.numCh,res);
           }
           else
           {
              LayerRK::get(ch->setting.numCh,res);
              value = res;
           }
              rk->value = res;

           }else 
           {
               if(ch->setting.ioCh == 0)
                   value = rk->value;
               else
                   rk->value = value;
           }
               
            break;
        }
        case E_CH_ADC:
        {
            break;
        }
        case E_CH_DAC:
        {
            //! сохранить данные в канале
            TDesDAC * dac = (TDesDAC*)ch->desData;
            dac->volt = (uint16_t)math_func::roundMin((value + 10.0)/20.0 * 8191);
            break;
        }
        case E_CH_DACWAV:
        {
            break;
        }
        case E_CH_MIL:
        {
            convParam<LayerOuMIL>(idCh,idParam,value);
//            if(ch->setting.ioCh == 1)
//                LayerOuMIL::set(ch->idCh,idParam);
//            else
//                LayerOuMIL::get(ch->idCh,idParam);

            break;
        }
        case E_CH_AR:
        {
            convParam<LayerArinc>(idCh,idParam,value);
            //if()
            break;
        }
//        case E_GEN_U:
//        {
//            //! сохранить данные в канале
//            TDesGen_U * gen_u = (TDesGen_U*)ch->desData;
//            
//            uint16_t u = (uint16_t)(value.u);
//            gen_u->u = ((uint16_t)round((double)0x3ff / (double)140 * u))) & 0x3ff;
//            gen_u->f = ((uint16_t)round(value.f * 67.108864) - 20000) & 0x7fff;
//            gen_u->ff=  (uint16_t)trunc((double)15000 / value.ff) & 0x7fff;
//            gen_u->phase = value.phase;
//            break;
//        }
//        case E_GEN_NU:
//        {
//            //! сохранить данные в канале
//            TDesGen_NU * gen_nu = (TDesGen_NU*)ch->desData;
//            gen_nu->f  = (uint32_t)round(value.f * 67.108864) - 20000;
//            gen_nu->ff = (uint16_t)trunc((double)15000 / value.ff) & 0x7fff;
//            break;
//        }
        case E_IR:
        {
            //! сохранить данные в канале
            TDesIR * ir = (TDesIR*)ch->desData;
            ir->value = (uint16_t) math_func::roundMin(4095*(ir->max - value)/(ir->max - ir->min));
            break;
        }
        case E_IP:
        {
            TDesIR * ip = (TDesIR*)ch->desData;
            ip->value = (uint16_t)math_func::roundMin(4095 * (value/100));
            break;
        }
        case E_ITP:
        {
            TDesITP *itp = (TDesITP*)ch->desData;
            itp->value   = (int16_t)math_func::roundMin(3760*(value/itp->max));
            break;
        }
        default:{}
    };
    return true;
}
template <> bool doHAL<TDesGen_U>(int32_t idCh,    //! идентификатор канала
                                 uint32_t idParam, //! идентификатор параметра
                                 TDesGen_U &value);

template <> bool doHAL<TDesGen_NU>(int32_t idCh,    //! идентификатор канала
                                 uint32_t idParam, //! идентификатор параметра
                                 TDesGen_NU &value);

template <typename T> bool doHAL(int32_t idCh,    //! идентификатор канала
                                 T &value)
{
    return doHAL<T>(idCh,0,value);
}
template <typename T> bool faultHAL(int32_t idCh,
                                    T fault)
{
    TCh *ch = &(HAL::obj()->chTable.ch[idCh]);

    if(ch == 0 )
        return false;
    if(ch->typeNode != HAL::obj()->typeCurrentNode ||
            ch->idNode   != HAL::obj()->currentNode )
    {
        //! тогда помещаем в буфер для передачи по сети
           
        return false;
    }
    switch(ch->typeCh)
    {
    case E_CH_RK:
    {       
        break;
    }
    case E_CH_ADC:
    {
        break;
    }case E_CH_DAC:
    {
        break;
    }
    case E_CH_DACWAV:
    {
        break;
    }
    case E_CH_MIL:
    {
        
        break;
    }
    case E_CH_AR:
    {
        //LayerAr::switchAr(idCh,)
        break;
    }
    default:{}
    };
    return true;   
}
template <typename T, typename Y> bool ioctlHAL(uint32_t idCh, T command, Y* dataOut)
{
    TCh *ch = &(HAL::obj()->chTable.ch[idCh]);

    if(ch == 0 )
        return false;
    switch(ch->typeCh)
    {
    case E_CH_MIL:
            {
                if(command == IO_CHECK_KC_MIL)
                    *dataOut = LayerOuMIL::checkKC(idCh);
                break;
            }
    
    };
    
    return true;
}
template <typename T, typename Y> bool ioctlHAL(uint32_t idCh, T command, Y dataIn)
{
    TCh *ch = &(HAL::obj()->chTable.ch[idCh]);

    if(ch == 0 )
        return false;
    if(ch->typeNode != HAL::obj()->typeCurrentNode ||
       ch->idNode   != HAL::obj()->currentNode )
    {
       switch(ch->typeCh)
       {
            case E_CH_MIL:
            {
                TDesMIL *mil = (TDesMIL*)(ch->desData);
                mil->active = dataIn;
                break;
            }
            case E_CH_AR:
            {
                TDesAr *ar = (TDesAr*)(ch->desData);
                ar->active = dataIn;
                break;
            }
            default:{}

       };
       //! тогда помещаем в буфер для передачи по сети
       return false;
    }
    switch(ch->typeCh)
    {
        case E_CH_RK:
        {       
            break;
        }
        case E_CH_ADC:
        {
            break;
        }case E_CH_DAC:
        {
            break;
        }
        case E_CH_DACWAV:
        {
            break;
        }
        case E_CH_MIL:
        {
            TDesMIL *mil = (TDesMIL*)(ch->desData);
            mil->active = dataIn;
            ch->setting.activeLocal = dataIn;
            if(command == IO_SWITCH_ON_OFF)
                LayerOuMIL::enable(ch->setting.numCh, mil->addr,ch->setting.numAdapter,dataIn);
//            else if(command == IO_CHECK_KC_MIL)
//                *dataOut = LayerOuMIL::checkKC(idCh));
            break;
        }
        case E_CH_AR:
        {
            if(command == IO_SWITCH_ON_OFF)
            {
                TDesAr *ar = (TDesAr*)(ch->desData);
                LayerArinc::switchAr(ch->setting.numCh,ch->setting.ioCh,dataIn);
                ar->active = dataIn;
                ch->setting.activeLocal = dataIn;
                return true;
            }else if(command == IO_SEND_DATA)
            {
                LayerArinc::switchAr(ch->setting.numCh,ch->setting.ioCh,1);
                return true;
            }
            break;
        }
        default:{}
        };
        return false;       
}
//! копирование данных и канальных адресов из одного канала в другой
bool copyChHAL(int32_t idFrom, int32_t idTo);

#endif
