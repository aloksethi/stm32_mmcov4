/*
 * sensor_handler.c
 *
 *  Created on: 24.2.2021
 *      Author: asethi
 */

#include "main.h"

static TaskHandle_t	g_handle_sensor_task;

void vSensorHandlerTask(void * pvParameters)
{
	while(1)
	{
		vTaskDelay(1000);

	}
	return;
}

void vStartSensorTask(UBaseType_t uxPriority )
{
	BaseType_t xReturned;

	/* Spawn the task. */
	xReturned = xTaskCreate( vSensorHandlerTask, "SENSOR", SENSOR_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) &g_handle_sensor_task );
	if( xReturned != pdPASS )
	{
		trace_printf("failed to create sensor handling task\n");
		Error_Handler();
	}

	return;
}
