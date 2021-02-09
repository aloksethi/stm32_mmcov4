/*
 * heartbeat.c
 *
 *  Created on: Jan 25, 2021
 *      Author: asethi
 */

#include "main.h"

static portTASK_FUNCTION( vLEDFlashTask, pvParameters )
{
	TickType_t xFlashRate, xLastFlashTime;


	/* The parameters are not used. */
	( void ) pvParameters;

	xFlashRate = LED_FLASH_RATE_BASE;
	xFlashRate /= portTICK_PERIOD_MS;

	/* We will turn the LED on and off again in the delay period, so each
	delay is only half the total period. */
	xFlashRate /= ( TickType_t ) 2;

	/* We need to initialise xLastFlashTime prior to the first call to
	vTaskDelayUntil(). */
	xLastFlashTime = xTaskGetTickCount();

	for(;;)
	{

		vTaskDelayUntil( &xLastFlashTime, xFlashRate );
		//board_red_led_toggle();
		//board_blue_led_toggle();
		board_green_led_toggle();
		vTaskDelayUntil( &xLastFlashTime, xFlashRate );
		//board_red_led_toggle();
		//board_blue_led_toggle();
		board_green_led_toggle();
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */


void vStartLEDFlashTasks( UBaseType_t uxPriority )
{
	BaseType_t xReturned;

	xReturned = xTaskCreate( vLEDFlashTask, "LEDx", LED_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
	if( xReturned != pdPASS )
	{
		/* The task was created.  Use the task's handle to delete the task. */
		trace_printf("failed to create the LED task\n");
	}
}
