/*
	FreeRTOS.org V5.0.0 - Copyright (C) 2003-2008 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
    * and even write all or part of your application on your behalf.          *
    * See http://www.OpenRTOS.com for details of the services we provide to   *
    * expedite your project.                                                  *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting,
	licensing and training services.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Fast Interrupt Test" - A high frequency periodic interrupt is generated
 * using a free running timer to demonstrate the use of the
 * configKERNEL_INTERRUPT_PRIORITY configuration constant.  The interrupt
 * service routine measures the number of processor clocks that occur between
 * each interrupt - and in so doing measures the jitter in the interrupt timing.
 * The maximum measured jitter time is latched in the ulMaxJitter variable, and
 * displayed on the LCD by the 'Check' task as described below.  The
 * fast interrupt is configured and handled in the timertest.c source file.
 *
 * "LCD" task - the LCD task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the LCD send the message on a queue to the LCD task instead of
 * accessing the LCD themselves.  The LCD task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" task -  This only executes every five seconds but has the highest
 * priority so is guaranteed to get processor time.  Its main function is to
 * check that all the standard demo tasks are still operational.  Should any
 * unexpected behaviour within a demo task be discovered the 'check' task will
 * write an error to the LCD (via the LCD task).  If all the demo tasks are
 * executing with their expected behaviour then the check task writes PASS
 * along with the max jitter time to the LCD (again via the LCD task), as
 * described above.
 */



/* Standard includes. */
//#include <stdio.h>
#include "printf.h"
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x.h"
#include "stm32f10x_it.h"

/* Demo app includes. */
#include "stm32_eval.h"
#include "stm3210e_eval_lcd.h"
#include "LCD_Message.h"
#include "death.h"
#include "integer.h"
#include "blocktim.h"
#include "partest.h"
#include "semtest.h"
#include "PollQ.h"
#include "flash.h"
#include "comtest2.h"

/*-----------------------------------------------------------*/

/* The time between cycles of the 'check' functionality (defined within the
tick hook. */
#define mainCHECK_DELAY						( ( portTickType ) 5000 / portTICK_RATE_MS )

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
//#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainCOM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )

/* The maximum number of message that can be waiting for display at any one
time. */
#define mainLCD_QUEUE_SIZE					( 3 )

/* The check task uses the sprintf function so requires a little more stack. */
#define mainCHECK_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 350 )
#define mainLCD_TASK_STACK_SIZE				( configMINIMAL_STACK_SIZE + 350 )

/* Dimensions the buffer into which the jitter time is written. */
#define mainMAX_MSG_LEN						25

/* The period of the system clock in nano seconds.  This is used to calculate
the jitter time in nano seconds. */
#define mainNS_PER_CLOCK					( ( unsigned portLONG ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Constants related to the LCD. */
#define mainMAX_LINE						( 240 )
#define mainROW_INCREMENT					( 24 )
#define mainMAX_COLUMN						( 20 )
#define mainCOLUMN_START					( 319 )
#define mainCOLUMN_INCREMENT 				( 16 )

/* Baud rate used by the comtest tasks. */
#define mainCOM_TEST_BAUD_RATE		( 115200 )

/* The LED used by the comtest tasks. See the comtest.c file for more
information. */
#define mainCOM_TEST_LED			( 3 )

/*-----------------------------------------------------------*/

/**
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/**
 * Configure the LCD as required by the demo.
 */
static void prvConfigureLCD( void );

/**
 * The LCD is written two by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the LCD directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */
static void vLCDTask( void *pvParameters );

/**
 * Checks the status of all the demo tasks then prints a message to the
 * display.  The message will be either PASS - and include in brackets the
 * maximum measured jitter time (as described at the to of the file), or a
 * message that describes which of the standard demo tasks an error has been
 * discovered in.
 *
 * Messages are not written directly to the terminal, but passed to vLCDTask
 * via a queue.
 */
static void vCheckTask( void *pvParameters );

/**
 * Configures the timers and interrupts for the fast interrupt test as
 * described at the top of this file.
 */
extern void vSetupTimerTest( void );

/**
 * External dependence needed by printf implementation. Write a character to standard out.
 *
 * @param c Specifies the character to be written.
 * @return Returns the character written. No error conditions are managed.
 */
int putChar( int ch );

/*-----------------------------------------------------------*/

/* The queue used to send messages to the LCD task. */
xQueueHandle xLCDQueue;

/*-----------------------------------------------------------*/

volatile int ITM_RxBuffer;

int putChar( int ch )
{
static unsigned portSHORT usColumn = 0, usRefColumn = mainCOLUMN_START;
static unsigned portCHAR ucLine = 0;

	if( ( usColumn == 0 ) && ( ucLine == 0 ) )
	{
		LCD_Clear(White);
	}

	if( ch != '\n' )
	{
		/* Display one character on LCD */
		LCD_DisplayChar( ucLine, usRefColumn, (u8) ch );

		/* Decrement the column position by 16 */
		usRefColumn -= mainCOLUMN_INCREMENT;

		/* Increment the character counter */
		usColumn++;
		if( usColumn == mainMAX_COLUMN )
		{
			ucLine += mainROW_INCREMENT;
			usRefColumn = mainCOLUMN_START;
			usColumn = 0;
		}
	}
	else
	{
		/* Move back to the first column of the next line. */
		ucLine += mainROW_INCREMENT;
		usRefColumn = mainCOLUMN_START;
		usColumn = 0;
	}

	/* Wrap back to the top of the display. */
	if( ucLine >= mainMAX_LINE )
	{
		ucLine = 0;
	}

	return ch;
}

void printOnLCD(char *msg){
	int nrChars = strlen(msg);
	int i;

	for(i=0 ; i<nrChars ; i++){
		putChar(msg[i]);
	}
}


int main( void )
{
#ifdef DEBUG
  debug();
#endif


	prvSetupHardware();

	/* Create the queue used by the LCD task.  Messages for display on the LCD
	are received via this queue. */
	xLCDQueue = xQueueCreate( mainLCD_QUEUE_SIZE, sizeof( xLCDMessage ) );


	/* Start the standard demo tasks. */
    vCreateBlockTimeTasks();
    vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
    vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );
    vStartIntegerMathTasks( mainINTEGER_TASK_PRIORITY );
	vStartLEDFlashTasks( mainFLASH_TASK_PRIORITY );
	vAltStartComTestTasks( mainCOM_TEST_PRIORITY, mainCOM_TEST_BAUD_RATE, mainCOM_TEST_LED );

	/* Start the tasks defined within this file/specific to this demo. */
    xTaskCreate( vCheckTask, ( signed portCHAR * ) "Check", mainCHECK_TASK_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );
	xTaskCreate( vLCDTask, ( signed portCHAR * ) "LCD", mainLCD_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );

	/* The suicide tasks must be created last as they need to know how many
	tasks were running prior to their creation in order to ascertain whether
	or not the correct/expected number of tasks are running at any given time. */
    vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );

	/* Configure the timers used by the fast interrupt timer test. */
	vSetupTimerTest();

	/* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
	return 0;
}
/*-----------------------------------------------------------*/

void vLCDTask( void *pvParameters )
{
xLCDMessage xMessage;

	/* Initialise the LCD and display a startup message. */
	prvConfigureLCD();
	LCD_DrawMonoPict( ( unsigned portLONG * ) pcBitmap );

	for( ;; )
	{
		/* Wait for a message to arrive that requires displaying. */
		while( xQueueReceive( xLCDQueue, &xMessage, portMAX_DELAY ) != pdPASS );

		/* Display the message.  Print each message to a different position. */
		printf( ( portCHAR const * ) xMessage.pcMessage );
		//printOnLCD( ( portCHAR const * ) xMessage.pcMessage );
	}
}
/*-----------------------------------------------------------*/

static void vCheckTask( void *pvParameters )
{
portTickType xLastExecutionTime;
xLCDMessage xMessage;
static signed portCHAR cPassMessage[ mainMAX_MSG_LEN ];
extern unsigned portSHORT usMaxJitter;

	xLastExecutionTime = xTaskGetTickCount();
	xMessage.pcMessage = cPassMessage;

    for( ;; )
	{
		/* Perform this check every mainCHECK_DELAY milliseconds. */
		vTaskDelayUntil( &xLastExecutionTime, mainCHECK_DELAY );

		/* Has an error been found in any task? */

		if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN BLOCK TIME\n";
		}
        else if( xAreSemaphoreTasksStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR IN SEMAPHORE\n";
        }
        else if( xArePollingQueuesStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR IN POLL Q\n";
        }
        else if( xIsCreateTaskStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR IN CREATE\n";
        }
        else if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR IN MATH\n";
        }
		else
		{
			sprintf( ( portCHAR * ) cPassMessage, "PASS [%uns]\n", ( ( unsigned portLONG ) usMaxJitter ) * mainNS_PER_CLOCK );
		}

		/* Send the message to the LCD gatekeeper for display. */
		xQueueSend( xLCDQueue, &xMessage, portMAX_DELAY );
	}
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE );

	/* SPI2 Periph clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );


	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

	vParTestInitialise();
}
/*-----------------------------------------------------------*/

static void prvConfigureLCD( void )
{
	/* Initialize the LCD */
	STM3210E_LCD_Init();

	/* Clear the LCD */
	LCD_Clear(White);


#ifdef LCD_MB542
GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure LCD Back Light (PA8) as output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	/* Set the Backlight Pin */
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);

	/* Initialize the LCD */
	LCD_Init();

	/* Set the Back Color */
	LCD_SetBackColor( White );

	/* Set the Text Color */
	LCD_SetTextColor( 0x051F );

	LCD_Clear();
#endif //LCD_MB5422
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	for( ;; );
}

#ifdef DEBUG

/* Keep the linker happy. */
void assert_failed( unsigned portCHAR* pcFile, unsigned portLONG ulLine )
{
	for( ;; )
	{
	}
}

#endif
