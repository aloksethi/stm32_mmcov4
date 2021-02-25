/*
 * pot_handler.c
 *
 *  Created on: 24.2.2021
 *      Author: asethi
 */

#include "main.h"

#define I2C_POT_1V_ADDRESS		(0x58 | (0x0 <<1))
#define I2C_POT_1V_CHANNEL		0

QueueHandle_t g_pot_queue_handle;

static TaskHandle_t g_handle_pot_task;
/* The variable used to hold the queue's data structure. */
static StaticQueue_t pot_queue_ds;
static uint8_t pot_queue_storage_area[POT_QUEUE_LENGTH
		* sizeof(pot_queue_data_t)];

static HAL_StatusTypeDef i2c_ll_set_pot(uint8_t addr, uint8_t channel,
		uint8_t val)
{
	HAL_StatusTypeDef ret1 = HAL_ERROR;
	;
	uint8_t data[2];

	data[0] = (uint8_t) ((channel & 0x1) << 7);
	data[1] = val;

	//if(xSemaphoreTake(g_mutex_i2c_op, portMAX_DELAY ) == pdTRUE )
	{
		ret1 = HAL_I2C_Master_Transmit(&g_hi2c1, addr, data, sizeof(data), 1);
		//	xSemaphoreGive(g_mutex_i2c_op);
	}

	if (HAL_OK != ret1)
	{
		trace_printf("failed to set pot %x \n", ret1);
		ret1 = HAL_ERROR;
	}
	else
	{
		trace_printf("success in setting pot\n");
		ret1 = HAL_OK;
	}
	return ret1;
}

void vPotHandlerTask(void *pvParameters)
{
	pot_queue_data_t pot_queue_local_copy;

	while (1)
	{
		if (xQueueReceive(g_pot_queue_handle, &pot_queue_local_copy,
		portMAX_DELAY) == pdPASS)
		{
			switch (pot_queue_local_copy.command_code)
			{

			case G_UC_PB_SUP_EN:
			{
				uint8_t id, status;
				id = pot_queue_local_copy.pot_data.pot_id;
				status = pot_queue_local_copy.pot_data.pot_status;

				switch (id)
				{

				case 1:
				{
					if (status)
						board_pb_sup1_en();
					else
						board_pb_sup1_dis();

					break;
				}
				case 2:
				{
					if (status)
						board_pb_sup2_en();
					else
						board_pb_sup2_dis();

					break;
				}
				case 3:
				{
					if (status)
						board_pb_sup3_en();
					else
						board_pb_sup3_dis();

					break;
				}
				case 4:
				{
					if (status)
						board_pb_sup4_en();
					else
						board_pb_sup4_dis();

					break;
				}
				case 5:
				{
					if (status)
						board_pb_sup5_en();
					else
						board_pb_sup5_dis();

					break;
				}
				case 6:
				{
					if (status)
						board_pb_lcl5v_en();
					else
						board_pb_lcl5v_dis();

					break;
				}
				}

				break;
			}

			case G_UC_PB_SUP_VAL:
			{
				HAL_StatusTypeDef ret1;
				uint8_t addr, channel, val;

				addr = (uint8_t) I2C_POT_1V_ADDRESS;
				channel = (uint8_t) I2C_POT_1V_CHANNEL;
				ret1 = i2c_ll_set_pot(addr, channel, val);

				break;
			}

			}

		}
	}
	return;
}

void vStartPotTask(UBaseType_t uxPriority)
{
	BaseType_t xReturned;
	g_pot_queue_handle = xQueueCreateStatic(POT_QUEUE_LENGTH,
			sizeof(pot_queue_data_t), pot_queue_storage_area, &pot_queue_ds);
	if ( NULL == g_pot_queue_handle)
	{
		trace_printf("failed to create the pot queue\n");
		Error_Handler();
	}
	/* Spawn the task. */
	xReturned = xTaskCreate(vPotHandlerTask, "POT", POT_STACK_SIZE, NULL,
			uxPriority, (TaskHandle_t*) &g_handle_pot_task);
	if (xReturned != pdPASS)
	{
		trace_printf("failed to create the pot handling task\n");
		Error_Handler();
	}

	return;
}
