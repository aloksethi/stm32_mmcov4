/*
 * prog_chip_reg.h
 *
 *  Created on: 1.2.2021
 *      Author: asethi
 */

#ifndef INC_PROG_CHIP_REG_H_
#define INC_PROG_CHIP_REG_H_

#include "mlab_nucleo_if.h"


#define PC_QUEUE_LENGTH    64

#define NUM_REG_BITS  		(__builtin_ffs(G_MAX_NUM_REGS) - 1)




typedef struct __attribute__((packed))
{
	uint8_t command_code;
	uint8_t reg_id;
	uint8_t ic_id;
} pc_queue_data_t;

void pc_get_curr_value(uint8_t , reg_t * );
void pc_set_curr_value(uint8_t , reg_t * );
void pc_save_default_value(uint8_t , reg_t * );
void pc_save_mini_value(uint8_t , reg_t * );
void pc_save_maxi_value(uint8_t , reg_t * );


void vStartChipRegTask(UBaseType_t );


#endif /* INC_PROG_CHIP_REG_H_ */
