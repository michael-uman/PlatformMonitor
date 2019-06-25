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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile int counter = 0;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

osThreadId_t flashTaskHandle;
osThreadId_t uartTaskHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void startFlashTask(void *argument); // for v2
void startUartTask(void *argument); // for v2

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

char msgBuffer[128];

void print2Uart2(const char *msg)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
}

/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)
	stdarg version contributed by Christian Ettinger

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Changes for the FreeRTOS ports:

	- The dot in "%-8.8s"
	- The specifiers 'l' (long) and 'L' (long long)
	- The specifier 'u' for unsigned
	- Dot notation for IP addresses:
	  sprintf("IP = %xip\n", 0xC0A80164);
      will produce "IP = 192.168.1.100\n"
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"

#define PAD_RIGHT 1
#define PAD_ZERO 2

/* You can define you own version of this character-output function. */
void __attribute__((weak)) vOutputChar( const char cChar, const TickType_t xTicksToWait  )
{
}

static const TickType_t xTicksToWait = pdMS_TO_TICKS( 20 );

struct xPrintFlags
{
	int base;
	int width;
	int printLimit;
	unsigned
		pad : 8,
		letBase : 8,
		isSigned : 1,
		isNumber : 1,
		long32 : 1,
		long64 : 1;
};

struct SStringBuf
{
	char *str;
	const char *orgStr;
	const char *nulPos;
	int curLen;
	struct xPrintFlags flags;
};

static void strbuf_init( struct SStringBuf *apStr, char *apBuf, const char *apMaxStr )
{
	apStr->str = apBuf;
	apStr->orgStr = apBuf;
	apStr->nulPos = apMaxStr-1;
	apStr->curLen = 0;

	memset( &apStr->flags, '\0', sizeof apStr->flags );
}
/*-----------------------------------------------------------*/

static BaseType_t strbuf_printchar( struct SStringBuf *apStr, int c )
{
	if( apStr->str == NULL )
	{
		vOutputChar( ( char ) c, xTicksToWait );
		apStr->curLen++;
		return pdTRUE;
	}
	if( apStr->str < apStr->nulPos )
	{
		*( apStr->str++ ) = c;
		apStr->curLen++;
		return pdTRUE;
	}
	if( apStr->str == apStr->nulPos )
	{
		*( apStr->str++ ) = '\0';
	}
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static portINLINE BaseType_t strbuf_printchar_inline( struct SStringBuf *apStr, int c )
{
	if( apStr->str == NULL )
	{
		vOutputChar( ( char ) c, xTicksToWait );
		if( c == 0 )
		{
			return pdFALSE;
		}
		apStr->curLen++;
		return pdTRUE;
	}
	if( apStr->str < apStr->nulPos )
	{
		*(apStr->str++) = c;
		if( c == 0 )
		{
			return pdFALSE;
		}
		apStr->curLen++;
		return pdTRUE;
	}
	if( apStr->str == apStr->nulPos )
	{
		*( apStr->str++ ) = '\0';
	}
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static portINLINE int i2hex( int aCh )
{
int iResult;

	if( aCh < 10 )
	{
		iResult = '0' + aCh;
	}
	else
	{
		iResult = 'A' + aCh - 10;
	}

	return iResult;
}
/*-----------------------------------------------------------*/

static BaseType_t prints(struct SStringBuf *apBuf, const char *apString )
{
	register int padchar = ' ';
	int i,len;

	if( apBuf->flags.width > 0 )
	{
		register int len = 0;
		register const char *ptr;
		for( ptr = apString; *ptr; ++ptr )
		{
			++len;
		}

		if( len >= apBuf->flags.width )
		{
			apBuf->flags.width = 0;
		}
		else
		{
			apBuf->flags.width -= len;
		}

		if( apBuf->flags.pad & PAD_ZERO )
		{
			padchar = '0';
		}
	}
	if( ( apBuf->flags.pad & PAD_RIGHT ) == 0 )
	{
		for( ; apBuf->flags.width > 0; --apBuf->flags.width )
		{
			if( strbuf_printchar( apBuf, padchar ) == 0 )
			{
				return pdFALSE;
			}
		}
	}
	if( ( apBuf->flags.isNumber == pdTRUE ) && ( apBuf->flags.pad == pdTRUE ) )
	{
		/* The string to print represents an integer number.
		 * In this case, printLimit is the min number of digits to print
		 * If the length of the number to print is less than the min nb of i
		 * digits to display, we add 0 before printing the number
		 */
		len = strlen( apString );

		if( len < apBuf->flags.printLimit )
		{
			i = apBuf->flags.printLimit - len;
			for( ; i; i-- )
			{
				if( strbuf_printchar( apBuf, '0' )  == 0 )
				{
					return pdFALSE;
				}
			}
		}
	}
	/* The string to print is not the result of a number conversion to ascii.
	 * For a string, printLimit is the max number of characters to display
	 */
	for( ; apBuf->flags.printLimit && *apString ; ++apString, --apBuf->flags.printLimit )
	{
		if( !strbuf_printchar( apBuf, *apString ) )
		{
			return pdFALSE;
		}
	}

	for( ; apBuf->flags.width > 0; --apBuf->flags.width )
	{
		if( !strbuf_printchar( apBuf, padchar ) )
		{
			return pdFALSE;
		}
	}

	return pdTRUE;
}
/*-----------------------------------------------------------*/

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12	/* to print 4294967296 */

#if	SPRINTF_LONG_LONG
#warning 64-bit libraries will be included as well
static BaseType_t printll( struct SStringBuf *apBuf, long long i )
{
	char print_buf[ 2 * PRINT_BUF_LEN ];
	register char *s;
	register int t, neg = 0;
	register unsigned long long u = i;
	lldiv_t lldiv_result;

/* typedef struct
 * {
 * 	long long int quot; // quotient
 * 	long long int rem;  // remainder
 * } lldiv_t;
 */

	apBuf->flags.isNumber = pdTRUE;	/* Parameter for prints */
	if( i == 0LL )
	{
		print_buf[ 0 ] = '0';
		print_buf[ 1 ] = '\0';
		return prints( apBuf, print_buf );
	}

	if( ( apBuf->flags.isSigned == pdTRUE ) && ( apBuf->flags.base == 10 ) && ( i < 0LL ) )
	{
		neg = 1;
		u = -i;
	}

	s = print_buf + sizeof print_buf - 1;

	*s = '\0';
	/* 18446744073709551616 */
	while( u != 0 )
	{
		lldiv_result = lldiv( u, ( unsigned long long ) apBuf->flags.base );
		t = lldiv_result.rem;
		if( t >= 10 )
		{
			t += apBuf->flags.letBase - '0' - 10;
		}
		*( --s ) = t + '0';
		u = lldiv_result.quot;
	}

	if( neg != 0 )
	{
		if( ( apBuf->flags.width != 0 ) && ( apBuf->flags.pad & PAD_ZERO ) )
		{
			if( !strbuf_printchar( apBuf, '-' ) )
			{
				return pdFALSE;
			}
			--apBuf->flags.width;
		}
		else
		{
			*( --s ) = '-';
		}
	}

	return prints( apBuf, s );
}
#endif	/* SPRINTF_LONG_LONG */
/*-----------------------------------------------------------*/

static BaseType_t printi( struct SStringBuf *apBuf, int i )
{
	char print_buf[ PRINT_BUF_LEN ];
	register char *s;
	register int t, neg = 0;
	register unsigned int u = i;
	register unsigned base = apBuf->flags.base;

	apBuf->flags.isNumber = pdTRUE;	/* Parameter for prints */

	if( i == 0 )
	{
		print_buf[ 0 ] = '0';
		print_buf[ 1 ] = '\0';
		return prints( apBuf, print_buf );
	}

	if( ( apBuf->flags.isSigned == pdTRUE ) && ( base == 10 ) && ( i < 0 ) )
	{
		neg = 1;
		u = -i;
	}

	s = print_buf + sizeof print_buf - 1;

	*s = '\0';
	switch( base )
	{
	case 16:
		while( u != 0 )
		{
			t = u & 0xF;
			if( t >= 10 )
			{
				t += apBuf->flags.letBase - '0' - 10;
			}
			*( --s ) = t + '0';
			u >>= 4;
		}
		break;

	case 8:
	case 10:
		/* GCC compiles very efficient */
		while( u )
		{
			t = u % base;
			*( --s ) = t + '0';
			u /= base;
		}
		break;
/*
	// The generic case, not yet in use
	default:
		while( u )
		{
			t = u % base;
			if( t >= 10)
			{
				t += apBuf->flags.letBase - '0' - 10;
			}
			*( --s ) = t + '0';
			u /= base;
		}
		break;
*/
	}

	if( neg != 0 )
	{
		if( apBuf->flags.width && (apBuf->flags.pad & PAD_ZERO ) )
		{
			if( strbuf_printchar( apBuf, '-' ) == 0 )
			{
				return pdFALSE;
			}
			--apBuf->flags.width;
		}
		else
		{
			*( --s ) = '-';
		}
	}

	return prints( apBuf, s );
}
/*-----------------------------------------------------------*/

static BaseType_t printIp(struct SStringBuf *apBuf, unsigned i )
{
	char print_buf[16];

	sprintf( print_buf, "%u.%u.%u.%u",
		i >> 24,
		( i >> 16 ) & 0xff,
		( i >> 8 ) & 0xff,
		i & 0xff );
	apBuf->flags.isNumber = pdTRUE;	/* Parameter for prints */
	prints( apBuf, print_buf );

	return pdTRUE;
}
/*-----------------------------------------------------------*/

static void tiny_print( struct SStringBuf *apBuf, const char *format, va_list args )
{
	char scr[2];

	for( ; ; )
	{
		int ch = *( format++ );

		if( ch != '%' )
		{
			do
			{
				/* Put the most like flow in a small loop */
				if( strbuf_printchar_inline( apBuf, ch ) == 0 )
				{
					return;
				}
				ch = *( format++ );
			} while( ch != '%' );
		}
		ch = *( format++ );
		/* Now ch has character after '%', format pointing to next */

		if( ch == '\0' )
		{
			break;
		}
		if( ch == '%' )
		{
			if( strbuf_printchar( apBuf, ch ) == 0 )
			{
				return;
			}
			continue;
		}
		memset( &apBuf->flags, '\0', sizeof apBuf->flags );

		if( ch == '-' )
		{
			ch = *( format++ );
			apBuf->flags.pad = PAD_RIGHT;
		}
		while( ch == '0' )
		{
			ch = *( format++ );
			apBuf->flags.pad |= PAD_ZERO;
		}
		if( ch == '*' )
		{
			ch = *( format++ );
			apBuf->flags.width = va_arg( args, int );
		}
		else
		{
			while( ch >= '0' && ch <= '9' )
			{
				apBuf->flags.width *= 10;
				apBuf->flags.width += ch - '0';
				ch = *( format++ );
			}
		}
		if( ch == '.' )
		{
			ch = *( format++ );
			if( ch == '*' )
			{
				apBuf->flags.printLimit = va_arg( args, int );
				ch = *( format++ );
			}
			else
			{
				while( ch >= '0' && ch <= '9' )
				{
					apBuf->flags.printLimit *= 10;
					apBuf->flags.printLimit += ch - '0';
					ch = *( format++ );
				}
			}
		}
		if( apBuf->flags.printLimit == 0 )
		{
			apBuf->flags.printLimit--;  /* -1: make it unlimited */
		}
		if( ch == 's' )
		{
			register char *s = ( char * )va_arg( args, int );
			if( prints( apBuf, s ? s : "(null)" ) == 0 )
			{
				break;
			}
			continue;
		}
		if( ch == 'c' )
		{
			/* char are converted to int then pushed on the stack */
			scr[0] = ( char ) va_arg( args, int );

			if( strbuf_printchar( apBuf, scr[0] )  == 0 )
			{
				return;
			}

			continue;
		}
		if( ch == 'l' )
		{
			ch = *( format++ );
			apBuf->flags.long32 = 1;
			/* Makes not difference as u32 == long */
		}
		if( ch == 'L' )
		{
			ch = *( format++ );
			apBuf->flags.long64 = 1;
			/* Does make a difference */
		}
		apBuf->flags.base = 10;
		apBuf->flags.letBase = 'a';

		if( ch == 'd' || ch == 'u' )
		{
			apBuf->flags.isSigned = ( ch == 'd' );
#if	SPRINTF_LONG_LONG
			if( apBuf->flags.long64 != pdFALSE )
			{
				if( printll( apBuf, va_arg( args, long long ) ) == 0 )
				{
					break;
				}
			} else
#endif	/* SPRINTF_LONG_LONG */
			if( printi( apBuf, va_arg( args, int ) ) == 0 )
			{
				break;
			}
			continue;
		}

		apBuf->flags.base = 16;		/* From here all hexadecimal */

		if( ch == 'x' && format[0] == 'i' && format[1] == 'p' )
		{
			format += 2;	/* eat the "xi" of "xip" */
			/* Will use base 10 again */
			if( printIp( apBuf, va_arg( args, int ) ) == 0 )
			{
				break;
			}
			continue;
		}
		if( ch == 'x' || ch == 'X' || ch == 'p' || ch == 'o' )
		{
			if( ch == 'X' )
			{
				apBuf->flags.letBase = 'A';
			}
			else if( ch == 'o' )
			{
				apBuf->flags.base = 8;
			}
#if	SPRINTF_LONG_LONG
			if( apBuf->flags.long64 != pdFALSE )
			{
				if( printll( apBuf, va_arg( args, long long ) ) == 0 )
				{
					break;
				}
			} else
#endif	/* SPRINTF_LONG_LONG */
			if( printi( apBuf, va_arg( args, int ) ) == 0 )
			{
				break;
			}
			continue;
		}
	}
	strbuf_printchar( apBuf, '\0' );
}
/*-----------------------------------------------------------*/

int tiny_printf( const char *format, ... )
{
va_list args;

	va_start( args, format );
	struct SStringBuf strBuf;
	strbuf_init( &strBuf, NULL, ( const char* )NULL );
	tiny_print( &strBuf, format, args );
	va_end( args );

	return strBuf.curLen;
}
/*-----------------------------------------------------------*/

int vsnprintf( char *apBuf, size_t aMaxLen, const char *apFmt, va_list args )
{
	struct SStringBuf strBuf;
	strbuf_init( &strBuf, apBuf, ( const char* )apBuf + aMaxLen );
	tiny_print( &strBuf, apFmt, args );

	return strBuf.curLen;
}
/*-----------------------------------------------------------*/

int snprintf( char *apBuf, size_t aMaxLen, const char *apFmt, ... )
{
	va_list args;

	va_start( args,  apFmt );
	struct SStringBuf strBuf;
	strbuf_init( &strBuf, apBuf, ( const char* )apBuf + aMaxLen );
	tiny_print( &strBuf, apFmt, args );
	va_end( args );

	return strBuf.curLen;
}
/*-----------------------------------------------------------*/

int sprintf( char *apBuf, const char *apFmt, ... )
{
	va_list args;

	va_start( args,  apFmt );
	struct SStringBuf strBuf;
	strbuf_init( &strBuf, apBuf, ( const char * )apBuf + 1024 );
	tiny_print( &strBuf, apFmt, args );
	va_end( args );

	return strBuf.curLen;
}
/*-----------------------------------------------------------*/

int vsprintf( char *apBuf, const char *apFmt, va_list args )
{
	struct SStringBuf strBuf;
	strbuf_init( &strBuf, apBuf, ( const char* ) apBuf + 1024 );
	tiny_print( &strBuf, apFmt, args );

	return strBuf.curLen;
}
/*-----------------------------------------------------------*/

const char *mkSize (unsigned long long aSize, char *apBuf, int aLen)
{
static char retString[33];
size_t gb, mb, kb, sb;

	if (apBuf == NULL) {
		apBuf = retString;
		aLen = sizeof retString;
	}
	gb = aSize / (1024*1024*1024);
	aSize -= gb * (1024*1024*1024);
	mb = aSize / (1024*1024);
	aSize -= mb * (1024*1024);
	kb = aSize / (1024);
	aSize -= kb * (1024);
	sb = aSize;
	if( gb )
	{
		snprintf (apBuf, aLen, "%u.%02u GB", ( unsigned ) gb, ( unsigned ) ( ( 100 * mb ) / 1024ul ) );
	}
	else if( mb )
	{
		snprintf (apBuf, aLen, "%u.%02u MB", ( unsigned ) mb, ( unsigned ) ( ( 100 * kb) / 1024ul ) );
	}
	else if( kb != 0ul )
	{
		snprintf (apBuf, aLen, "%u.%02u KB", ( unsigned ) kb, ( unsigned ) ( ( 100 * sb) / 1024ul ) );
	}
	else
	{
		snprintf (apBuf, aLen, "%u bytes", ( unsigned ) sb);
	}
	return apBuf;
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  osKernelInitialize(); // Initialize CMSIS-RTOS

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  print2Uart2("Initializing threads...\n");

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
    .stack_size = 256
  };
  uartTaskHandle = osThreadNew(startUartTask, NULL, &uartTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  print2Uart2("Starting kernel...\n");
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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
  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      //osDelay(10);
	  vTaskDelay(500 / portTICK_PERIOD_MS);
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
  /* Infinite loop */
//  static char msgBuffer[128];
  for(;;)
  {
	snprintf(msgBuffer, sizeof(msgBuffer), "Buffer (%04d)\n", counter);
//	snprintf(msgBuffer, sizeof(msgBuffer) - 1, "Buffer (%04d)\n", counter);
//	HAL_UART_Transmit_IT(&huart2, (uint8_t*)msgBuffer, strlen(msgBuffer));
//	osDelay(100);
	print2Uart2(msgBuffer);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  /* USER CODE END startUartTask */
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
