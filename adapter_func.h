#ifndef ADAPTER_FUNC_H
#define ADAPTER_FUNC_H
#include <stdint.h>
//! преобразование ЦАП16
uint16_t dac16ToCode(float u);//u - В

//! преобразование потенциометра
uint16_t ipToCode(float u);

//! преобразование для имитатора сопротивлений
uint16_t irToCode(float r, float min, float max);

//! преобразование для термопары
uint16_t itpToCode(float u,float max);

//! преобразование для генератора трехфазного напряжения
uint16_t genToCode();

#endif
