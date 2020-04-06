#include "glHAL.hpp"


template <> bool doHAL<TDesGen_U>(int32_t idCh,    //! идентификатор канала
                                 uint32_t idParam, //! идентификатор параметра
                                 TDesGen_U &value)
{
    if(idCh==-1)
          return false;
    TCh *ch = &(HAL::obj()->chTable.ch[idCh]);

     if(ch == 0 )
         return false;
     //! данные готовы для чтения или для записи(нужно сравнивать с предыдущим занчением)
     ch->setting.readyToWrite = 1;
     
     switch(ch->typeCh)
     {
         case E_GEN_U:
         {
             //! сохранить данные в канале
             TDesGen_U * gen_u = (TDesGen_U*)ch->desData;
             
             
             gen_u->u = ((int16_t)math_func::roundMin((double)0x3ff / 150. * value.u)) & 0x3ff;
             gen_u->f = ((uint16_t)(math_func::roundMin(value.f * 67.108864) - 20000)) & 0x7fff;
             gen_u->ff=  (uint16_t)math_func::truncVal((double)15000. / (double)value.f) & 0x7fff;
             gen_u->phase = value.phase;
             break;
         }       
         default:{}
     };
     return true;

}
template <> bool doHAL<TDesGen_NU>(int32_t idCh,    //! идентификатор канала
                                 uint32_t idParam, //! идентификатор параметра
                                 TDesGen_NU &value)
{
    if(idCh == -1)
        return false;
    TCh *ch = &(HAL::obj()->chTable.ch[idCh]);

     if(ch == 0 )
         return false;
     //! данные готовы для чтения или для записи(нужно сравнивать с предыдущим занчением)
     ch->setting.readyToWrite = 1;
     
     switch(ch->typeCh)
     {
     case E_GEN_NU:
            {
                //! сохранить данные в канале
                TDesGen_NU * gen_nu = (TDesGen_NU*)ch->desData;
                gen_nu->f  = (uint32_t)math_func::roundMin(value.f * 67.108864) - 20000;
                gen_nu->ff = (uint16_t)math_func::truncVal((double)15000 / value.f) & 0x7fff;
                break;
            }
         default:{}
     };
     return true;

}
bool copyChHAL(int32_t idFrom, int32_t idTo)
{
    if(idFrom == -1 || idTo == -1)
        return false;
    TCh *chFrom = &(HAL::obj()->chTable.ch[idFrom]);
    TCh *chTo   = &(HAL::obj()->chTable.ch[idTo]);
    if(chFrom == 0 || chTo == 0)
        return false;

    if(chFrom->typeCh != chTo->typeCh)
        return false;

    /*if(ch->typeNode != HAL::obj()->typeCurrentNode ||
       ch->idNode   != HAL::obj()->currentNode )
    {
       //! тогда помещаем в буфер для передачи по сети
       return false;
    }*/

    switch(chFrom->typeCh)
    {

        case E_CH_AR:
        {
            
            TDesAr *arFrom = (TDesAr*)(chFrom->desData);
            TDesAr *arTo   = (TDesAr*)(chTo->desData);
            chTo->setting.readyToWrite = 1;
            for(int i=0;i<arFrom->numAddr;i++)
            {
                arTo->value[i] = arFrom->value[i];
            }
            break;
        }
    };
    return true;
}
