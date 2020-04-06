#include "adapter_func.h"
#include <math.h>
//! преобразование ЦАП16
uint16_t dac16ToCode(float u)//u - В
{
    return (uint16_t)floor(u + 10.0/20.0 * 8191  + 0.5);
}
//! преобразование потенциометра
uint16_t ipToCode(float u)
{
    return (uint16_t)floor((4095 * u/100.) + 0.5);
}
//! преобразование для имитатора сопротивлений
uint16_t irToCode(float r, float min, float max)
{
    return (uint16_t) floor((4095 *(max - r)/(max - min)) + 0.5);
}
//! преобразование для термопары
uint16_t itpToCode(float u,float max)
{
    return (uint16_t) floor((u * 3760 / max) + 0.5 );
}
//! преобразование для генератора трехфазного напряжения
uint16_t genToCode()
{
    return 0;
}
