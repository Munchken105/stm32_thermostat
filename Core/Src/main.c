/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SHT41_ADDR (0x44U << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

uint8_t TX_Buffer[1] = {0xFD}; //data to send 0xFD is high precision mode list of commands can be found: https://cdn-shop.adafruit.com/product-files/5776/Datasheet_SHT4x.pdf : in section 4.5
uint8_t RX_Buffer[6];
int temp_valid;
int humidity_valid;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static float calculate_temp(uint16_t raw_temp, int mode);
static float calculate_humidity(uint16_t raw_humidity);
static uint8_t crc_calc(uint8_t *data, uint8_t length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //I2C  for 0x44

	  HAL_StatusTypeDef receive;
	  HAL_StatusTypeDef transmit;

	  float temp_f;
	  float humidity_f;

	  transmit = HAL_I2C_Master_Transmit(&hi2c1,SHT41_ADDR, TX_Buffer, 1, HAL_MAX_DELAY); //transmit the data to the sensor

	  if (transmit != HAL_OK){
		  temp_valid = 0;
		  humidity_valid = 0;

		  printf("Transition failed\r\n");

		  HAL_Delay(100);
		  continue;
	  }

	  HAL_Delay(100); //delay

	  receive = HAL_I2C_Master_Receive(&hi2c1, SHT41_ADDR, (uint8_t *)RX_Buffer, 6, HAL_MAX_DELAY); //read the data from the sensor

	  if (receive != HAL_OK){
		  temp_valid = 0;
		  humidity_valid = 0;

		  printf("Receiving failed\r\n");
		  HAL_Delay(100);
		  continue;
	  }

	  uint16_t raw_temp = ((uint16_t)RX_Buffer[0] << 8) | RX_Buffer[1]; //temp shift left 8 to make it MSB than add LSB by oring it
	  uint16_t raw_humidity = ((uint16_t)RX_Buffer[3] << 8) | RX_Buffer[4]; //humidity shift left 8 to make it MSB than add LSB by oring it



	  uint8_t crc_temp = crc_calc(&RX_Buffer[0],2);

	  uint8_t crc_humidity = crc_calc(&RX_Buffer[3],2);

	  if (crc_temp == RX_Buffer[2]){
		  temp_valid = 1;
		  temp_f = calculate_temp(raw_temp, 1);
	  }
	  else{
		  temp_valid = 0;
		  printf("Temperature CRC failed\r\n");
	  }

	  if (crc_humidity == RX_Buffer[5]){
		  humidity_valid = 1;
		  humidity_f = calculate_humidity(raw_humidity);
	  }
	  else{
		  humidity_valid = 0;
		  printf("Humidity CRC failed\r\n");
	  }

	  if (temp_valid == 1 && humidity_valid == 1){
		  printf("Temperature: %.1f F\r\n",temp_f);
		  printf("Humidity: %.1f %%\r\n", humidity_f);
	  }

	  HAL_Delay(100);
  }

    /* USER CODE END WHILE */


    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV4;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00402D41;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

	static float calculate_humidity(uint16_t raw_humidity){
		float Hi = (float)raw_humidity/65535.0f;

		float H = -6.0f + (125.0f * Hi);

		return H;

	}

	static uint8_t crc_calc(uint8_t *data, uint8_t length){ //length in bytes https://cdn-shop.adafruit.com/product-files/5776/Datasheet_SHT4x.pdf section 4.4 and 4.3 are useful //CRC checks are modulo 2 division from what i understand doing research

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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
