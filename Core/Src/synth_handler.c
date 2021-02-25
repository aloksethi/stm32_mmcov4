/*
 * synth_handler.c
 *
 *  Created on: 24.2.2021
 *      Author: asethi
 */
#include "main.h"

static TaskHandle_t	g_handle_synth_task;

void vSynthHandlerTask(void * pvParameters)
{
	pc_queue_data_t pc_queue_local_copy;

	while(1)
	{
		vTaskDelay(1000);

	}
	return;
}

void vStartSynthTask(UBaseType_t uxPriority )
{
	BaseType_t xReturned;

	/* Spawn the task. */
	xReturned = xTaskCreate( vSynthHandlerTask, "SYNTH", SYNTH_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) &g_handle_synth_task );
	if( xReturned != pdPASS )
	{
		trace_printf("failed to create the synth handling task\n");
		Error_Handler();
	}

	return;
}
