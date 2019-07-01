/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "task.h"
#include "FreeRTOS.h"
#include "tiny_printf.h"
#include "commands.h"
#include "firmware_version.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/**
 * Global application context structure
 */
typedef struct {
	osMutexId_t contextMutex;
	uint32_t counter;
	uint32_t led1 :1, led2 :1, led3 :1, led4 :1;
	uint32_t but1 :1, but2 :1, but3 :1, but4 :1;
	uint32_t error;
} APP_CONTEXT;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

typedef StaticQueue_t osStaticMessageQDef_t;
osThreadId_t flashTaskHandle;
osThreadId_t uartTaskHandle;
osThreadId_t recvTaskHandle;
osMessageQueueId_t sendQueueHandle;
uint8_t sendQueueBuffer[ 16 * sizeof( SENDMSGQUEUE_OBJ_t ) ];
osStaticMessageQDef_t sendQueueControlBlock;
osMessageQueueId_t recvQueueHandle;
uint8_t recvQueueBuffer[ 16 * sizeof( RECVMSGQUEUE_OBJ_t ) ];
osStaticMessageQDef_t recvQueueControlBlock;
/* USER CODE BEGIN PV */

volatile APP_CONTEXT appContext = { 0 };

uint8_t gRecvBuffer[sizeof(RECVCMD_t)];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
void startFlashTask(void *argument); // for v2
void startUartTask(void *argument); // for v2
void startRecvTask(void *argument); // for v2

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
	for (;;) {
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_Delay(100);
	}
}
#endif

void print2Uart2(const char *msg) {
	osMutexAcquire(appContext.contextMutex, 0);
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), 100);
	osMutexRelease(appContext.contextMutex);
}

/**
 * Callback is called when the receive from UART is complete.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	static uint32_t counter = 0;
	RECVMSGQUEUE_OBJ_t msg;

	memcpy(&msg.packet, gRecvBuffer, sizeof(RECVCMD_t));
	msg.count = counter++;

	osStatus result = osMessageQueuePut(recvQueueHandle, (void *) &msg,
			sizeof(msg), 0);
	if (result != osOK) {
		// TODO: Should handle error condition here...
	}
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  /* Alarm generation: Turn LED1 on */
//  BSP_LED_On(LED1);
//	osMutexAcquire(appContext.contextMutex, 0);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
	appContext.led4 = 1;
//	osMutexRelease(appContext.contextMutex);
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
  MX_USART2_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  // Send a bunch of crs so if frontend is listening it will sync easily
  print2Uart2("\n\n\n\n\n\n\n\n");
  /* USER CODE END 2 */

  osKernelInitialize(); // Initialize CMSIS-RTOS

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	appContext.contextMutex = osMutexNew(NULL);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of sendQueue */
  const osMessageQueueAttr_t sendQueue_attributes = {
    .name = "sendQueue",
    .cb_mem = &sendQueueControlBlock,
    .cb_size = sizeof(sendQueueControlBlock),
    .mq_mem = &sendQueueBuffer,
    .mq_size = sizeof(sendQueueBuffer)
  };
  sendQueueHandle = osMessageQueueNew (16, sizeof(SENDMSGQUEUE_OBJ_t), &sendQueue_attributes);

  /* definition and creation of recvQueue */
  const osMessageQueueAttr_t recvQueue_attributes = {
    .name = "recvQueue",
    .cb_mem = &recvQueueControlBlock,
    .cb_size = sizeof(recvQueueControlBlock),
    .mq_mem = &recvQueueBuffer,
    .mq_size = sizeof(recvQueueBuffer)
  };
  recvQueueHandle = osMessageQueueNew (16, sizeof(RECVMSGQUEUE_OBJ_t), &recvQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */

	print2Uart2("{ \"message\": \"Initializing threads...\" }\n");

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of flashTask */
  const osThreadAttr_t flashTask_attributes = {
    .name = "flashTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 256
  };
  flashTaskHandle = osThreadNew(startFlashTask, NULL, &flashTask_attributes);

  /* definition and creation of uartTask */
  const osThreadAttr_t uartTask_attributes = {
    .name = "uartTask",
    .priority = (osPriority_t) osPriorityLow,
    .stack_size = 512
  };
  uartTaskHandle = osThreadNew(startUartTask, NULL, &uartTask_attributes);

  /* definition and creation of recvTask */
  const osThreadAttr_t recvTask_attributes = {
    .name = "recvTask",
    .priority = (osPriority_t) osPriorityRealtime,
    .stack_size = 256
  };
  recvTaskHandle = osThreadNew(startRecvTask, NULL, &recvTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	HAL_UART_Receive_IT(&huart2, (uint8_t*) &gRecvBuffer, sizeof(RECVCMD_t));

	// Push out version packet...
	{
		SENDMSGQUEUE_OBJ_t	msg = {0};

		msg.cmd = SENDCMD_VERSION;
//		msg.count = 0;
//		msg.buttonState = 0;

		osMessageQueuePut(sendQueueHandle, &msg, 0, 0);
	}

	print2Uart2("{ \"message\": \"Starting kernel...\" }\n");
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A 
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x15;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LD3_Pin LD4_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LD3_Pin|LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD5_Pin */
  GPIO_InitStruct.Pin = LD5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD5_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_startFlashTask */
/**
 * @brief  Function implementing the flashTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_startFlashTask */
void startFlashTask(void *argument)
{

  /* USER CODE BEGIN 5 */
	static uint32_t counter = 0;

	SENDMSGQUEUE_OBJ_t msg = { 0 };

	/* Infinite loop */
	for (;;) {
		int button = (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == 0) ? 1 : 0;
//	  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

		msg.cmd 		= SENDCMD_STATUS;
		msg.count 		= counter;
		msg.buttonState = button;

		osMessageQueuePut(sendQueueHandle, &msg, 0, 0);
		vTaskDelay(250 / portTICK_PERIOD_MS);
		counter++;
	}
  /* USER CODE END 5 */ 
}

/* USER CODE BEGIN Header_startUartTask */
/**
 * @brief Function implementing the uartTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startUartTask */
void startUartTask(void *argument)
{
  /* USER CODE BEGIN startUartTask */
	SENDMSGQUEUE_OBJ_t msg = { 0 };

	/* Infinite loop */
	static char msgBuffer[128];
	for (;;) {
		osStatus_t status;
		status = osMessageQueueGet(sendQueueHandle, &msg, NULL, 0);
		if (status == osOK) {
			osMutexAcquire(appContext.contextMutex, 0);
			switch (msg.cmd) {
			case SENDCMD_STATUS: {
				RTC_TimeTypeDef stimestructureget;
				RTC_DateTypeDef sdatestructureget;

				/* Get the RTC current Time */
                 HAL_RTC_GetTime(&hrtc, &stimestructureget, FORMAT_BCD);
                 HAL_RTC_GetDate(&hrtc, &sdatestructureget, FORMAT_BCD);

                uint32_t ledMask = ((appContext.led4 << 3) | (appContext.led3 << 2) | (appContext.led2 << 1) | (appContext.led1 << 0));

				appContext.counter = msg.count;
				appContext.but1 = (msg.buttonState & 0x1) ? 1 : 0;

				snprintf(msgBuffer, sizeof(msgBuffer),
						"{ \"id\": %ld, \"button\": %d, \"led\": %ld, \"timestamp\": \"%02x:%02x:%02x\" }\n",
						appContext.counter, appContext.but1, ledMask,
						stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
				print2Uart2(msgBuffer);
			}
				break;
			case SENDCMD_VERSION: {
				snprintf(msgBuffer, sizeof(msgBuffer),
						"{ \"version\": \"%d.%d.%d\" }\n",
						FW_MAJOR_VERSION, FW_MINOR_VERSION, FW_VERSION_BUILD);
				print2Uart2(msgBuffer);
			}
				break;
			default:
				break;
			}
			osMutexRelease(appContext.contextMutex);
			vTaskDelay(100);
		}
	}
  /* USER CODE END startUartTask */
}

/* USER CODE BEGIN Header_startRecvTask */
/**
 * @brief Function implementing the recvTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startRecvTask */
void startRecvTask(void *argument)
{
  /* USER CODE BEGIN startRecvTask */
	/* Infinite loop */
	for (;;) {
		RECVMSGQUEUE_OBJ_t msg;
		osStatus_t status = osMessageQueueGet(recvQueueHandle, &msg, NULL, 0);
		if (status == osOK) {
#if 0
			HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin);
#endif
			switch (msg.packet.cmd) {
			case RECVCMD_HELLO:
				break;
			case RECVCMD_LEDON:
				{
					osMutexAcquire(appContext.contextMutex, 0);
					if (IS_LED_SET(msg.packet.data, LED_ONE)) {
						appContext.led1 = 1;
						HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
					} else if (IS_LED_SET(msg.packet.data, LED_TWO)) {
						appContext.led2 = 1;
						HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
					} else if (IS_LED_SET(msg.packet.data, LED_THREE)) {
						appContext.led3 = 1;
						HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
					} else if (IS_LED_SET(msg.packet.data, LED_FOUR)) {
						appContext.led4 = 1;
						HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
					} else {

					}
					osMutexRelease(appContext.contextMutex);
				}
				break;
			case RECVCMD_LEDOFF:
				{
					osMutexAcquire(appContext.contextMutex, 0);
					if (IS_LED_SET(msg.packet.data, LED_ONE)) {
						appContext.led1 = 0;
						HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
					} else if (IS_LED_SET(msg.packet.data, LED_TWO)) {
						appContext.led2 = 0;
						HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
					} else if (IS_LED_SET(msg.packet.data, LED_THREE)) {
						appContext.led3 = 0;
						HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);
					} else if (IS_LED_SET(msg.packet.data, LED_FOUR)) {
						appContext.led4 = 0;
						HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
					} else {

					}
					osMutexRelease(appContext.contextMutex);
				}
				break;
			case RECVCMD_VERSION: {
				SENDMSGQUEUE_OBJ_t msg = { 0 };
				msg.cmd = SENDCMD_VERSION;
				osStatus_t putStat = osMessageQueuePut(sendQueueHandle, &msg, 0,
						0);
				if (putStat != osOK) {
					appContext.error = 1;
				}
			}
				break;
			case RECVCMD_END:
				break;
			}


		} else {

		}
		// Queue up another read from the UART peripheral
		HAL_UART_Receive_IT(&huart2, (uint8_t*) &gRecvBuffer,
				sizeof(RECVCMD_t));
		osDelay(10);
	}
  /* USER CODE END startRecvTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
