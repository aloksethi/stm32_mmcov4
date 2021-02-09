/*
 * prog_chip_reg.c
 *
 *  Created on: 1.2.2021
 *      Author: asethi
 */

#include "main.h"
#include "semphr.h"
#include <string.h>

/* The variable used to hold the queue's data structure. */
static StaticQueue_t pc_queue_ds;
static StaticSemaphore_t pc_mutex_ds;

static pc_queue_data_t pc_queue_local_copy;
static reg_t pc_tmp_local_copy; // should be local but stack might be small

SemaphoreHandle_t pc_mutex_handle = NULL;


/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
static uint8_t pc_queue_storage_area[ PC_QUEUE_LENGTH * sizeof(pc_queue_data_t) ];

static reg_t pc_curr_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];
static reg_t pc_dflt_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];
static reg_t pc_mini_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];
static reg_t pc_maxi_reg_dbase[G_MAX_ICS_PER_UC][G_MAX_NUM_REGS];

void pc_get_curr_value(uint8_t ic_id, reg_t * local_copy)
{
	if( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE )
	    {
			uint8_t reg_id = local_copy->reg_id;
			local_copy->cascade = pc_curr_reg_dbase[ic_id-1][reg_id].cascade;
			strncpy((char *)local_copy->reg_val, (const char *)pc_curr_reg_dbase[ic_id-1][reg_id].reg_val, sizeof(local_copy->reg_val));

	        xSemaphoreGive( pc_mutex_handle );
	    }
		return;
}

void pc_set_curr_value(uint8_t ic_id, reg_t * local_copy)
{
	if( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE )
	    {
			uint8_t reg_id = local_copy->reg_id;
			pc_curr_reg_dbase[ic_id-1][reg_id].cascade = local_copy->cascade;
			strncpy((char *)pc_curr_reg_dbase[ic_id-1][reg_id].reg_val, (const char *)local_copy->reg_val, sizeof(local_copy->reg_val));

	        xSemaphoreGive( pc_mutex_handle );
	    }
		return;
}

void pc_save_default_value()
{
    if( xSemaphoreTake( pc_mutex_handle, portMAX_DELAY ) == pdTRUE )
    {
        xSemaphoreGive( pc_mutex_handle );
    }
	return;
}

/*
 * reg_num is the register number-->6bit in mmcov4, 5 bit in older chips
 * buff-->pointer to the raw hex data, had to change it from uint64_t
 * cause it seems that there are more than 3 shift registers in cascade
 * num_data_bytes-->num of bytes, should be multiple of 5 as one register is 5 bytes
 * in the buff-->msb is at lowest address and lsb is at the highest address
 */
static void pc_program_bits(uint8_t ic_id, reg_t * tmp_reg)
{
	uint8_t ii;
	uint8_t i;
	uint8_t tmp_data;
	uint8_t tot_bits = tmp_reg->cascade * G_LENGTH_ONE_REG;
	uint8_t bits_txmtd = 0;
	uint8_t num_data_bytes;

	num_data_bytes = tmp_reg->cascade * G_LENGTH_ONE_REG>>3;
	num_data_bytes += (tmp_reg->cascade % 2);

	// shift in data fist,
	// lsb goes in first
	for ( i=num_data_bytes; i>0; i-- )
	{
		tmp_data = tmp_reg->reg_val[i-1];
		for ( ii=0; ii<sizeof(uint8_t); ii++)
		{
			uint8_t val;

			val = (tmp_data & 0x1<<ii)>>ii;
			board_set_reg_data(val);
			board_send_reg_clock();
			bits_txmtd++;
			if (bits_txmtd >= tot_bits)
				break;
		}
	}
	//send the LE after some extra delay
	for( i=0; i<30; i++ );

	if (ic_id == 1)
		board_send_le_ic1();
	else if (ic_id == 2)
		board_send_le_ic2();
	else
		trace_printf("Wrong IC ID\n");

	return;
}


void vChipHandlerTask(void * pvParameters)
{

	while(1)
	{
		if( xQueueReceive( g_pc_queue_handle, &pc_queue_local_copy, portMAX_DELAY ) == pdPASS )
		{
			switch (pc_queue_local_copy.command_code)
			{
			case G_UC_SET_REG_CONFIG:
			{
				pc_tmp_local_copy.reg_id = pc_queue_local_copy.reg_id;
				pc_get_curr_value(pc_queue_local_copy.ic_id, &pc_tmp_local_copy);

				pc_program_bits(pc_queue_local_copy.ic_id, &pc_tmp_local_copy);
				break;
			}
			}
		}

	}

	return;
}

void vStartChipRegTask(UBaseType_t uxPriority )
{
	BaseType_t xReturned;
	g_pc_queue_handle = xQueueCreateStatic( PC_QUEUE_LENGTH, sizeof(reg_t), pc_queue_storage_area, &pc_queue_ds );
	if( NULL == g_pc_queue_handle )
	{
		trace_printf("failed to create the pc queue\n");
		Error_Handler();
	}
	pc_mutex_handle = xSemaphoreCreateMutexStatic( &pc_mutex_ds );
	if ( NULL == pc_mutex_handle )
	{
		trace_printf("failed to create mutex for register storage data\n");
		Error_Handler();
	}
	/* Spawn the task. */
	xReturned = xTaskCreate( vChipHandlerTask, "CHIP", CHIP_REG_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) &g_handle_chip_reg_task );
	if( xReturned != pdPASS )
	{
		trace_printf("failed to create the chip register handling task\n");
		Error_Handler();
	}

	return;
}
