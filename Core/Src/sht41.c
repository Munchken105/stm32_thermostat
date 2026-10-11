#include <stdio.h>
#include "sht41.h"
//Macros
#define SHT41_ADDR (0x44U << 1)

//private functions declartion
static float calculate_temp(uint16_t raw_temp, int mode);
static float calculate_humidity(uint16_t raw_humidity);
static uint8_t crc_calc(uint8_t *data, uint8_t length);

//public functions delcarations
int SHT41_Decode(const uint8_t data[6], int mode, float *temp, float *humidity);
HAL_StatusTypeDef SHT41_Read(I2C_HandleTypeDef *hi2c, float *temp, float *humidity);

//private functions
static float calculate_temp(uint16_t raw_temp, int mode){ //https://cdn-shop.adafruit.com/product-files/5776/Datasheet_SHT4x.pdf : section 4.6 // mode is 0 for Celsius and 1 for Fahrenheit.
	float Ti = (float)raw_temp/ 65535.0f;

	//Celsius
	if (mode == 0){
	float T = -45.0f + (175 * Ti);
	return T;
	}

	//Fahrenheit
	if (mode == 1){
		float T = -49.0 + (315.0f * Ti);
		return T;
	}

	return 0.0f;
}

//function calculating humidity
static float calculate_humidity(uint16_t raw_humidity){
	float Hi = (float)raw_humidity/65535.0f;

	float H = -6.0f + (125.0f * Hi);

	return H;

}

static uint8_t crc_calc(const uint8_t *data, uint8_t length){ //length in bytes https://cdn-shop.adafruit.com/product-files/5776/Datasheet_SHT4x.pdf section 4.4 and 4.3 are useful //CRC checks are modulo 2 division from what i understand doing research

	uint8_t crc = 0xFF;

	for(int i = 0; i < length; i++){

		crc  ^= data[i];

		for(uint8_t j = 0; j < 8; j++){ //8bits is a size of a byte thats why use less than 8

			if(crc & 0x80){
				crc = (crc << 1) ^ 0x31;
			}
			else{
				crc = (crc << 1);
			}

		}
	}
	crc ^= 0x00;

	return crc;
}


//public functions

HAL_StatusTypeDef SHT41_I2C(I2C_HandleTypeDef *hi2c, uint8_t data[6]){

	if (hi2c == NULL || data == NULL){
		return HAL_ERROR;
	}

	uint8_t TX_Buffer = 0xFD;//data to send 0xFD is high precision mode list of commands can be found: https://cdn-shop.adafruit.com/product-files/5776/Datasheet_SHT4x.pdf : in section 4.5

	HAL_StatusTypeDef transmit = HAL_I2C_Master_Transmit(hi2c, SHT41_ADDR, &TX_Buffer, 1, 100);

	if (transmit != HAL_OK){
		return transmit;
	}

	HAL_Delay(100);

	HAL_StatusTypeDef receive = HAL_I2C_Master_Receive(hi2c, SHT41_ADDR, data, 6, 100);

	return receive;
}


int SHT41_Decode(const uint8_t data[6], int mode, float *temp, float *humidity){
	//validate arguments
	if (data == NULL || temp == NULL || humidity == NULL){
		return 0;
	}

	if (mode != SHT41_CELSIUS && mode != SHT41_FAHRENHEIT){
		return 0;
	}

	//crc check
	if(crc_calc(&data[0], 2) != data[2] ){
		return 0;
	}

	if (crc_calc(&data[3], 2) != data[5]){
		return 0;
	}

	// combining sensor data
	uint16_t raw_temp = ((uint16_t)data[0] << 8) | data[1];
	uint16_t raw_humidity = ((uint16_t)data[3] << 8) | data[4];

	//calc values for temp and humidity

	*temp = calculate_temp(raw_temp, mode);
	*humidity = calculate_humidity(raw_humidity);

	return 1;
}

