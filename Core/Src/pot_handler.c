/*
 * pot_handler.c
 *
 *  Created on: 24.2.2021
 *      Author: asethi
 */

#include "main.h"

QueueHandle_t g_pot_queue_handle;

static TaskHandle_t g_handle_pot_task;
/* The variable used to hold the queue's data structure. */
static StaticQueue_t pot_queue_ds;
static uint8_t pot_queue_storage_area[POT_QUEUE_LENGTH
		* sizeof(pot_queue_data_t)];

static void pot_id_to_addr_map(uint8_t id, uint8_t *addr, uint8_t *channel)
{

	switch (id)
	{
	case G_PB_SUP1:
	{
		*addr = I2C_POT_SUP1_ADDRESS;
		*channel = I2C_POT_SUP1_CHANNEL;
		break;
	}
	case G_PB_SUP2:
	{
		*addr = I2C_POT_SUP2_ADDRESS;
		*channel = I2C_POT_SUP2_CHANNEL;
		break;
	}
	case G_PB_SUP3:
	{
		*addr = I2C_POT_SUP3_ADDRESS;
		*channel = I2C_POT_SUP3_CHANNEL;
		break;
	}
	case G_PB_SUP4:
	{
		*addr = I2C_POT_SUP4_ADDRESS;
		*channel = I2C_POT_SUP4_CHANNEL;
		break;
	}
	case G_PB_SUP5:
	{
		*addr = I2C_POT_SUP5_ADDRESS;
		*channel = I2C_POT_SUP5_CHANNEL;
		break;
	}

	}
	return;
}
static uint8_t i2c_ll_set_pot(uint8_t addr, uint8_t channel, uint8_t val)
{
	uint8_t data[2], ret_val;
	HAL_StatusTypeDef ret;

	data[0] = (uint8_t) ((channel & 0x1) << 7);
	data[1] = val;

	if (xSemaphoreTake(g_mutex_i2c_op, portMAX_DELAY) == pdTRUE)
	{
		ret = HAL_I2C_Master_Transmit(&g_hi2c1, addr, data, sizeof(data), 100);
		xSemaphoreGive(g_mutex_i2c_op);
	}
	if (ret == HAL_OK)
		ret_val = 0;
	else
	{
		trace_printf("i2c_op failed: %d\n", ret);
	}
	return ret_val;
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
				id = pot_queue_local_copy.pot_data.sup_id;
				status = pot_queue_local_copy.pot_data.sup_status;

				switch (id)
				{

				case G_PB_SUP1:
				{
					if (status)
						board_pb_sup1_en();
					else
						board_pb_sup1_dis();

					break;
				}
				case G_PB_SUP2:
				{
					if (status)
						board_pb_sup2_en();
					else
						board_pb_sup2_dis();

					break;
				}
				case G_PB_SUP3:
				{
					if (status)
						board_pb_sup3_en();
					else
						board_pb_sup3_dis();

					break;
				}
				case G_PB_SUP4:
				{
					if (status)
						board_pb_sup4_en();
					else
						board_pb_sup4_dis();

					break;
				}
				case G_PB_SUP5:
				{
					if (status)
						board_pb_sup5_en();
					else
						board_pb_sup5_dis();

					break;
				}
				case G_PB_SUP_LCL_5V:
				{
					if (status)
						board_pb_lcl5v_en();
					else
						board_pb_lcl5v_dis();

					break;
				}
				case G_PB_SUP_ALL:
				{
					if (status)
					{
						board_pb_lcl5v_en();
						board_pb_sup1_en();
						board_pb_sup2_en();
						board_pb_sup3_en();
						board_pb_sup4_en();
						board_pb_sup5_en();

					}
					else
					{
						board_pb_sup1_dis();
						board_pb_sup2_dis();
						board_pb_sup3_dis();
						board_pb_sup4_dis();
						board_pb_sup5_dis();
						board_pb_lcl5v_dis();
					}

					break;
				}
				}

				break;
			}

			case G_UC_PB_SUP_VAL:
			{
				uint8_t ret;
				uint8_t id, addr, channel, raw_val;
				id = pot_queue_local_copy.pot_data.sup_id;
				raw_val = pot_queue_local_copy.pot_data.pot_val;

				pot_id_to_addr_map(id, &addr, &channel);

				ret = i2c_ll_set_pot(addr, channel, raw_val);

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
