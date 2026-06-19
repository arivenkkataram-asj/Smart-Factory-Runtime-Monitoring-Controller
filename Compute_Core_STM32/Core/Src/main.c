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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
#define LED_ON(port,pin)  HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET)
#define LED_OFF(port,pin) HAL_GPIO_WritePin(port,pin,GPIO_PIN_SET)
#define BUZZER_ON()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET)
#define BUZZER_OFF() HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
#define BATTERY_MIN_VOLTAGE  2.3f
#define SOLAR_CHARGE_VOLTAGE 2.5f

#define TANK_FULL_LEVEL      50.0f
#define TANK_EMPTY_LEVEL     20.0f
#define TANK_LOW_LEVEL       40.0f
#define TANK_HIGH_LEVEL      53.0f
/* USER CODE BEGIN PFP */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t Read_ADC_Channel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, 100);

    uint32_t value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return value;
}
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
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  LED_ON(GPIOB,GPIO_PIN_0);
  HAL_Delay(300);

  LED_ON(GPIOB,GPIO_PIN_1);
  HAL_Delay(300);

  LED_ON(GPIOB,GPIO_PIN_10);
  HAL_Delay(300);

  LED_OFF(GPIOB,GPIO_PIN_11);
  HAL_Delay(300);

  LED_ON(GPIOC,GPIO_PIN_13);
  HAL_Delay(300);

  LED_OFF(GPIOB,GPIO_PIN_0);
  LED_OFF(GPIOB,GPIO_PIN_1);
  LED_OFF(GPIOB,GPIO_PIN_10);
  LED_ON(GPIOB,GPIO_PIN_11);
  LED_OFF(GPIOC,GPIO_PIN_13);
  uint32_t solar_adc;
  uint32_t battery_adc;
  uint32_t current_adc;
  uint32_t tank_adc;
  float solar_voltage;
  float battery_voltage;
  float pump_current;
  float tank_level;

  uint8_t pumpFeedback;
  uint8_t dryRun;

  uint8_t pumpStatus = 0;
  uint8_t chargingStatus = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  pumpFeedback = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);

	  dryRun = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);

	  solar_adc   = Read_ADC_Channel(ADC_CHANNEL_0);
	  battery_adc = Read_ADC_Channel(ADC_CHANNEL_1);
	  current_adc = Read_ADC_Channel(ADC_CHANNEL_2);
	  tank_adc    = Read_ADC_Channel(ADC_CHANNEL_3);

	  solar_voltage   = (solar_adc * 3.3f) / 4095.0f;

	  battery_voltage = (battery_adc * 3.3f) / 4095.0f;

	  pump_current    = (current_adc * 10.0f) / 4095.0f;   // Simulate 0-10A

	  tank_level      = (tank_adc * 100.0f) / 4095.0f;     // 0-100%
	  // Charging Logic
	  if(solar_voltage > SOLAR_CHARGE_VOLTAGE && battery_voltage < 3.2)
	  {
	      LED_ON(GPIOB, GPIO_PIN_1);      // Charging LED
	      LED_ON(GPIOC,GPIO_PIN_13);
	  }
	  else
	  {
	      LED_OFF(GPIOB, GPIO_PIN_1);
	      LED_ON(GPIOC,GPIO_PIN_13);
	  }

	  // Pump Control Logic

	  if(tank_level < TANK_LOW_LEVEL)
	  {
	      if(battery_voltage > BATTERY_MIN_VOLTAGE)
	      {
	          if(dryRun == 1)
	          {
	              // Start Pump

	              LED_ON(GPIOB, GPIO_PIN_0);
	              LED_ON(GPIOC,GPIO_PIN_13);
	              //HAL_Delay(2000);

	              if(pumpFeedback)
	              {
	                  // Normal Operation

	                  LED_OFF(GPIOB, GPIO_PIN_10);

	                  BUZZER_OFF();

	                  LED_ON(GPIOC, GPIO_PIN_13);
	                  LED_ON(GPIOC,GPIO_PIN_13);
	              }
	              else
	              {
	                  // Pump Feedback Fault

	                  LED_OFF(GPIOB, GPIO_PIN_0);

	                  LED_ON(GPIOB, GPIO_PIN_10);

	                  BUZZER_ON();
	                  LED_OFF(GPIOC,GPIO_PIN_13);
	              }
	          }
	          else
	          {
	              // Dry Run Fault

	              LED_OFF(GPIOB, GPIO_PIN_0);

	              LED_ON(GPIOB, GPIO_PIN_10);

	              BUZZER_ON();
	              LED_OFF(GPIOC,GPIO_PIN_13);
	          }
	      }
	      else
	      {
	          // Low Battery Fault

	          LED_OFF(GPIOB, GPIO_PIN_0);

	          LED_ON(GPIOB, GPIO_PIN_10);

	          BUZZER_ON();
	          LED_OFF(GPIOC,GPIO_PIN_13);
	      }
	  }
	  else if(tank_level >= TANK_HIGH_LEVEL)
	  {
	      // Tank Full

	      LED_OFF(GPIOB, GPIO_PIN_0);

	      LED_OFF(GPIOB, GPIO_PIN_10);

	      BUZZER_OFF();
	      LED_ON(GPIOC,GPIO_PIN_13);
	  }
	  printf("SV=%.2f,", solar_voltage);
	  printf("BV=%.2f,", battery_voltage);
	  printf("PC=%.2f,", pump_current);
	  printf("TL=%.1f,", tank_level);

	  printf("PF=%d,", pumpFeedback);
	  printf("DR=%d,", dryRun);

	  printf("PS=%d,", pumpStatus);
	  printf("CS=%d\r\n", chargingStatus);

	  HAL_Delay(3000);
    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB10 PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
