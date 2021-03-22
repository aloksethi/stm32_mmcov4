/*
 * prog_chip_reg.c
 *
 *  Created on: 1.2.2021
 *      Author: asethi
 */

#include "main.h"
#include "semphr.h"
#include <string.h>

SemaphoreHandle_t pc_mutex_handle = NULL;

/* The variable used to hold the queue's data structure. */
static StaticQueue_t pc_queue_ds;
static StaticSemaphore_t pc_mutex_ds;

static reg_t pc_tmp_local_copy; // should be local but stack might be small
static TaskHandle_t g_handle_chip_reg_task;

/* The array to use as the queue's storage area.  This must be at least
 uxQueueLength * uxItemSize bytes. */
static uint8_t pc_queue_storage_area[PC_QUEUE_LENGTH * sizeof(pc_queue_data_t)];

static reg_t pc_curr_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];
static reg_t pc_dflt_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];
static reg_t pc_mini_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];
static reg_t pc_maxi_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];

void pc_get_curr_value(uint8_t ic_id, reg_t *local_copy)
{
	if ( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE)
	{
		uint8_t reg_id = local_copy->reg_id;
		local_copy->cascade = pc_curr_reg_dbase[ic_id - 1][reg_id].cascade;
		memcpy((void*) local_copy->reg_val,
				(const void*) pc_curr_reg_dbase[ic_id - 1][reg_id].reg_val,
				sizeof(local_copy->reg_val));

		xSemaphoreGive(pc_mutex_handle);
	}
	return;
}

void pc_set_curr_value(uint8_t ic_id, reg_t *local_copy)
{
	if ( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE)
	{
		uint8_t reg_id = local_copy->reg_id;
		pc_curr_reg_dbase[ic_id - 1][reg_id].cascade = local_copy->cascade;
		memcpy((void*) pc_curr_reg_dbase[ic_id - 1][reg_id].reg_val,
				(const void*) local_copy->reg_val, sizeof(local_copy->reg_val));

		xSemaphoreGive(pc_mutex_handle);
	}
	return;
}

void pc_save_default_value(uint8_t ic_id, reg_t *local_copy)
{
	if ( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE)
	{
		uint8_t reg_id = local_copy->reg_id;
		pc_dflt_reg_dbase[ic_id - 1][reg_id].cascade = local_copy->cascade;
		trace_printf("size is %d\n", sizeof(local_copy->reg_val));
		memcpy((void*) pc_dflt_reg_dbase[ic_id - 1][reg_id].reg_val,
				(const void*) local_copy->reg_val, sizeof(local_copy->reg_val));

		xSemaphoreGive(pc_mutex_handle);
	}
	return;
}

void pc_save_mini_value(uint8_t ic_id, reg_t *local_copy)
{
	if ( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE)
	{
		uint8_t reg_id = local_copy->reg_id;
		pc_mini_reg_dbase[ic_id - 1][reg_id].cascade = local_copy->cascade;
		trace_printf("size is %d\n", sizeof(local_copy->reg_val));
		memcpy((void*) pc_mini_reg_dbase[ic_id - 1][reg_id].reg_val,
				(const void*) local_copy->reg_val, sizeof(local_copy->reg_val));

		xSemaphoreGive(pc_mutex_handle);
	}
	return;
}

void pc_save_maxi_value(uint8_t ic_id, reg_t *local_copy)
{
	if ( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE)
	{
		uint8_t reg_id = local_copy->reg_id;
		pc_maxi_reg_dbase[ic_id - 1][reg_id].cascade = local_copy->cascade;
		trace_printf("size is %d\n", sizeof(local_copy->reg_val));
		memcpy((void*) pc_maxi_reg_dbase[ic_id - 1][reg_id].reg_val,
				(const void*) local_copy->reg_val, sizeof(local_copy->reg_val));

		xSemaphoreGive(pc_mutex_handle);
	}
	return;
}

static void board_set_reg_data(uint8_t data)
{
	if (data)
		HAL_GPIO_WritePin(IC_DATA_Port, IC_DATA_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(IC_DATA_Port, IC_DATA_Pin, GPIO_PIN_RESET);
	return;
}

static void board_send_reg_clock(void)
{
	for (int i = 0; i < CHIP_CLK_DELAY; i++)
		;
	HAL_GPIO_WritePin(IC_CLK_Port, IC_CLK_Pin, GPIO_PIN_SET);

	for (int i = 0; i < CHIP_CLK_DELAY; i++)
		;
	HAL_GPIO_WritePin(IC_CLK_Port, IC_CLK_Pin, GPIO_PIN_RESET);

	return;
}

static void board_send_le_ic1(void)
{
	for (int i = 0; i < CHIP_CLK_DELAY; i++)
		;
	HAL_GPIO_WritePin(IC1_LE_Port, IC1_LE_Pin, GPIO_PIN_SET);

	for (int i = 0; i < CHIP_CLK_DELAY; i++)
		;
	HAL_GPIO_WritePin(IC1_LE_Port, IC1_LE_Pin, GPIO_PIN_RESET);
	return;
}

static void board_send_le_ic2(void)
{
	for (int i = 0; i < CHIP_CLK_DELAY; i++)
		;
	HAL_GPIO_WritePin(IC2_LE_Port, IC2_LE_Pin, GPIO_PIN_SET);

	for (int i = 0; i < CHIP_CLK_DELAY; i++)
		;
	HAL_GPIO_WritePin(IC2_LE_Port, IC2_LE_Pin, GPIO_PIN_RESET);
	return;
}

/*
 * reg_num is the register number-->10bit in mmcov4, 5 bit in older chips.
 * a9...a0, a0 to a4 are for LE1 to LE31, a5...a9 are for LE32 to LE62
 * buff-->pointer to the raw hex data, had to change it from uint64_t
 * cause it seems that there are more than 3 shift registers in cascade
 * num_data_bytes-->num of bytes, should be multiple of 5 as one register is 5 bytes
 * in the buff-->msb is at lowest address and lsb is at the highest address
 */
static void pc_program_bits(uint8_t ic_id, reg_t *tmp_reg)
{
	uint8_t ii;
	uint8_t i;
	uint8_t tmp_data;
	uint8_t tot_bits = tmp_reg->cascade * G_LENGTH_ONE_REG;
	uint8_t bits_txmtd = 0;
	uint8_t num_data_bytes;
	uint8_t reg_num = tmp_reg->reg_id;
	uint8_t high_reg_num, low_reg_num;

	num_data_bytes = tmp_reg->cascade * G_LENGTH_ONE_REG >> 3;
	num_data_bytes += (tmp_reg->cascade % 2);

	trace_printf("num_data_bytes:%d, reg_num:%d\n", num_data_bytes, reg_num);
	// shift in data fist,
	// lsb goes in first
	for (i = num_data_bytes; i > 0; i--)
	{
		/*
		 * max index 9, min idex 0, last data is saved at G_STORAGE_FOR_ONE_REG_BYTES-1
		 * i.e., data for hw bits b7..b0 is saved at reg_val[9], b8...b15 is at reg_val[8]
		 * and so on. data for b72..b79 will be at reg_val[0]
		 */
		tmp_data =
				tmp_reg->reg_val[(G_STORAGE_FOR_ONE_REG_BYTES - 1) - (i - 1)];
		for (ii = 0; ii < 8; ii++)
		{
			uint8_t val;

			val = (tmp_data & 0x1 << ii) >> ii;
			board_set_reg_data(val);
			board_send_reg_clock();
			bits_txmtd++;
			if (bits_txmtd >= tot_bits)
				break;
		}
	}
	// send address, as there are two address decoders, lower decoder handles regs 0 to 31 and
	// higher one handles regs 32 to 62. latch 0 is unused on both
	// ad0 goes first and ad9 goes last
	if (reg_num > 31)
	{
		high_reg_num = reg_num - 31;
		low_reg_num = 0;
	}
	else
	{
		high_reg_num = 0;
		low_reg_num = reg_num;
	}

	for (ii = 0; ii < 5; ii++)
	{
		uint8_t val;

		val = (low_reg_num & 0x1 << ii) >> ii;
		board_set_reg_data(val);
		board_send_reg_clock();
	}

	for (ii = 0; ii < 5; ii++)
	{
		uint8_t val;

		val = (high_reg_num & 0x1 << ii) >> ii;
		board_set_reg_data(val);
		board_send_reg_clock();
	}

	//send the LE after some extra delay
	for (i = 0; i < 30; i++)
		;

	if (ic_id == 1)
		board_send_le_ic1();
	else if (ic_id == 2)
		board_send_le_ic2();
	else
	{
		trace_printf("Wrong IC ID\n");
		Error_Handler();
	}

	return;
}

void vChipHandlerTask(void *pvParameters)
{
	pc_queue_data_t pc_queue_local_copy;

	while (1)
	{
		if (xQueueReceive(g_pc_queue_handle, &pc_queue_local_copy,
		portMAX_DELAY) == pdPASS)
		{
			if ((pc_queue_local_copy.ic_id == 0))
			{
				trace_printf("invalid message\n");
				Error_Handler();
			}
			switch (pc_queue_local_copy.command_code)
			{

			case G_UC_SET_REG_CONFIG:
			{
				if (pc_queue_local_copy.reg_id > G_MAX_NUM_REGS)
				{
					trace_printf("invalid message\n");
					Error_Handler();
				}

				pc_tmp_local_copy.reg_id = pc_queue_local_copy.reg_id;
				pc_get_curr_value(pc_queue_local_copy.ic_id,
						&pc_tmp_local_copy);

				pc_program_bits(pc_queue_local_copy.ic_id, &pc_tmp_local_copy);
				break;
			}

			case G_UC_APPLY_DEF_REG_CONFIG:
			{
				uint8_t ic_id = pc_queue_local_copy.ic_id;
				reg_t *local_ptr;
				uint8_t i;

				if ( xSemaphoreTake(pc_mutex_handle,
						portMAX_DELAY) == pdTRUE)
				{
					board_blue_led_toggle();

					for (i = 0; i < G_MAX_NUM_REGS; i++)
					{
						local_ptr = &(pc_dflt_reg_dbase[ic_id - 1][i]);
						pc_program_bits(pc_queue_local_copy.ic_id, local_ptr);
					}
					board_blue_led_toggle();

					xSemaphoreGive(pc_mutex_handle);
				}

				break;
			}

			case G_UC_APPLY_MAXI_REG_CONFIG:
			{
				uint8_t ic_id = pc_queue_local_copy.ic_id;
				reg_t *local_ptr;
				uint8_t i;

				if ( xSemaphoreTake(pc_mutex_handle,
						portMAX_DELAY) == pdTRUE)
				{
					board_blue_led_toggle();

					for (i = 0; i < G_MAX_NUM_REGS; i++)
					{
						local_ptr = &(pc_maxi_reg_dbase[ic_id - 1][i]);
						pc_program_bits(pc_queue_local_copy.ic_id, local_ptr);
					}
					board_blue_led_toggle();

					xSemaphoreGive(pc_mutex_handle);
				}

				break;
			}

			case G_UC_APPLY_MINI_REG_CONFIG:
			{
				uint8_t ic_id = pc_queue_local_copy.ic_id;
				reg_t *local_ptr;
				uint8_t i;

				if ( xSemaphoreTake(pc_mutex_handle,
						portMAX_DELAY) == pdTRUE)
				{
					board_blue_led_toggle();

					for (i = 0; i < G_MAX_NUM_REGS; i++)
					{
						local_ptr = &(pc_mini_reg_dbase[ic_id - 1][i]);
						pc_program_bits(pc_queue_local_copy.ic_id, local_ptr);
					}

					board_blue_led_toggle();

					xSemaphoreGive(pc_mutex_handle);
				}

				break;
			}
			}
		}

	}

	return;
}

void vStartChipRegTask(UBaseType_t uxPriority)
{
	BaseType_t xReturned;
	g_pc_queue_handle = xQueueCreateStatic(PC_QUEUE_LENGTH,
			sizeof(pc_queue_data_t), pc_queue_storage_area, &pc_queue_ds);
	if ( NULL == g_pc_queue_handle)
	{
		trace_printf("failed to create the pc queue\n");
		Error_Handler();
	}
	pc_mutex_handle = xSemaphoreCreateMutexStatic(&pc_mutex_ds);
	if ( NULL == pc_mutex_handle)
	{
		trace_printf("failed to create mutex for register storage data\n");
		Error_Handler();
	}
	/* Spawn the task. */
	xReturned = xTaskCreate(vChipHandlerTask, "CHIP",
	CHIP_REG_STACK_SIZE, NULL, uxPriority,
			(TaskHandle_t*) &g_handle_chip_reg_task);
	if (xReturned != pdPASS)
	{
		trace_printf("failed to create the chip register handling task\n");
		Error_Handler();
	}

	return;
}
