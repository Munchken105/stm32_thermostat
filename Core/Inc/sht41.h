#ifndef SHT41_H
#define SHT41_H

#include <stdint.h>
#include "stm32c0xx_hal.h"

//MACROS
#define SHT41_CELSIUS 0
#define SHT41_FAHRENHEIT 1


HAL_StatusTypeDef SHT41_I2C(I2C_HandleTypeDef *hi2c, uint8_t data[6]);

//calls the crc functions and temp humidity functions
int SHT41_Decode(const uint8_t data[6], int mode, float *temp, float *humidity);



#endif
