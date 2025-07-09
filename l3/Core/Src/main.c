/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EEPROM_ADDR 0b10100000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

UART_HandleTypeDef hlpuart1;
DMA_HandleTypeDef hdma_lpuart1_rx;
DMA_HandleTypeDef hdma_lpuart1_tx;

SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN PV */
uint8_t RxBuffer[50];
uint8_t TxBuffer[50];
uint8_t score = 0;
uint8_t eepromData[1];
uint8_t SPIRx[10];
uint8_t SPITx[10];
uint8_t state = 0;
uint32_t startTime = 0;
uint8_t LED;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI3_Init(void);
/* USER CODE BEGIN PFP */
void UARTConfig();
void WriteScore(uint8_t scorevalue);
uint8_t ReadScore();
void TurnOnLED();
void TurnOffLED();
void LightRandomLED();
bool CheckButtonPress();
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
  MX_DMA_Init();
  MX_LPUART1_UART_Init();
  MX_I2C1_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */
  uint8_t text[] = "Catch the Light!\r\nPress Button to Start\r\n\0";
  HAL_UART_Transmit(&hlpuart1, text, strlen(text), 10);
  UARTConfig();
  SPITxRx_Setup();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  switch(state){
	  case 0:
		  if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)){
			  TurnOnLED();
			  uint8_t starttext[] = "Game Started\r\n";
			  HAL_UART_Transmit(&hlpuart1, starttext, strlen(starttext), 10);
			  state = 1;
		  }
			  break;
		  case 1:
			  TurnOffLED();
			  startTime = HAL_GetTick();
			  score = 0;
			  LightRandomLED();
			  state = 2;
			  break;
		  case 2:
			  if ((HAL_GetTick()-startTime) >= 90000){
				  TurnOffLED();
				  WriteScore(score);
				  uint8_t endtext[] = "Game Finished\r\n";
				  HAL_UART_Transmit(&hlpuart1, endtext, strlen(endtext), 10);
				  sprintf((char*)TxBuffer,"Score : %d\r\n",score);
				  HAL_UART_Transmit_DMA(&hlpuart1, TxBuffer, strlen((char*)TxBuffer));
				  state = 0;
			  }
			  else
			  {
				  bool pressed = CheckButtonPress();
				  if(pressed){
					  score++;
					  TurnOffLED();
					  LightRandomLED();
				  }
			  }
			  break;
	  }
  }
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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
  hi2c1.Init.Timing = 0x30A0A7FB;
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
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 57600;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_9B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_EVEN;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void UARTConfig()
{
	HAL_UART_Receive_DMA(&hlpuart1, RxBuffer, 5);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &hlpuart1)
	{
		RxBuffer[5] = '\0';

		sprintf((char*)TxBuffer,"Received : %s\r\n",RxBuffer);
		HAL_UART_Transmit_DMA(&hlpuart1, TxBuffer, strlen((char*)TxBuffer));

		if (strstr((char*)RxBuffer, "score") != NULL){
			score = ReadScore();
			sprintf((char*)TxBuffer,"Score : %d\r\n",score);
			HAL_UART_Transmit_DMA(&hlpuart1, TxBuffer, strlen((char*)TxBuffer));
		}

		HAL_UART_Receive_DMA(&hlpuart1, RxBuffer, 5);
	}
}

void WriteScore(uint8_t scorevalue)
{
	if(hi2c1.State == HAL_I2C_STATE_READY){
		HAL_I2C_Mem_Write_IT(&hi2c1, EEPROM_ADDR, 0x00, I2C_MEMADD_SIZE_16BIT, &scorevalue, 1);
	}
}

uint8_t ReadScore()
{
	if(hi2c1.State == HAL_I2C_STATE_READY){
		HAL_I2C_Mem_Read_IT(&hi2c1, EEPROM_ADDR, 0x00, I2C_MEMADD_SIZE_16BIT, eepromData, 1);
	}
	return eepromData[0];
}

void SPITxRx_Setup()
{
//CS pulse
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 0); // CS Select
HAL_Delay(1);
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 1); // CS deSelect
HAL_Delay(1);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 1); //CS deSelect
}

void TurnOnLED(){
	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 0); // CS Select
	SPITx[0] = 0b01000000;
	SPITx[1] = 0x13;
	SPITx[2] = 0x0F;
	SPITx[3] = 0;
	HAL_SPI_TransmitReceive_IT(&hspi3, SPITx, SPIRx, 4);
	}
}

void TurnOffLED(){
	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 0); // CS Select
	SPITx[0] = 0b01000000;
	SPITx[1] = 0x13;
	SPITx[2] = 0x00;
	SPITx[3] = 0;
	HAL_SPI_TransmitReceive_IT(&hspi3, SPITx, SPIRx, 4);
	}
}

void LightRandomLED(){
	uint8_t LEDIndex = rand()%4;
	LED = (1 << LEDIndex);
	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 0); // CS Select
	SPITx[0] = 0b01000000;
	SPITx[1] = 0x13;
	SPITx[2] = LED;
	SPITx[3] = 0;
	HAL_SPI_TransmitReceive_IT(&hspi3, SPITx, SPIRx, 4);
	}
}

bool CheckButtonPress(){
	uint8_t button;
	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 0); // CS Select
	SPITx[0] = 0b01000001;
	SPITx[1] = 0x12;
	SPITx[2] = 0;
	SPITx[3] = 0;
	HAL_SPI_TransmitReceive_IT(&hspi3, SPITx, SPIRx, 4);
	}
	button = SPIRx[2];
	return (button & LED)?true:false;
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

#ifdef  USE_FULL_ASSERT
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
