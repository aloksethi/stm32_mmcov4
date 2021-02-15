/*
 * mlab_nucleo_if.h
 *
 *  Created on: 1.2.2021
 *      Author: asethi
 */

#ifndef INC_MLAB_NUCLEO_IF_H_
#define INC_MLAB_NUCLEO_IF_H_

#define G_MAX_ICS_PER_UC					2
#define G_REGS_PER_REG						4		// upto 4 shift registers are cascaded to make one shift register
#define G_LENGTH_ONE_REG					20 		//number of bits in one shift register
#define G_MAX_NUM_REGS						63		// two older muxes were cascaded, so one address went there
#define G_STORAGE_FOR_ONE_REG_BITS			((G_REGS_PER_REG * G_LENGTH_ONE_REG))  // this is in bits
#define G_STORAGE_FOR_ONE_REG_BYTES			(G_STORAGE_FOR_ONE_REG_BITS/8)

// command code, instead of enum have made it defines, easier to integrate in matlab
#define G_UC_MIN_COMMAND_CODE				0
#define G_UC_SET_REG_CONFIG					1
#define G_UC_SAVE_DEF_REG_CONFIG		 	2
#define G_UC_SAVE_MAXI_REG_CONFIG			3
#define G_UC_SAVE_MINI_REG_CONFIG 			4
#define G_UC_APPLY_DEF_REG_CONFIG			5
#define G_UC_APPLY_MAXI_REG_CONFIG			6
#define G_UC_APPLY_MINI_REG_CONFIG 			7
#define G_UC_SET_CURR_AS_DEF_CONFIG			8

#define G_UC_MAX_COMMAND_CODE				20

#define G_NUCLEO_PORT_NUM					4040

typedef struct __attribute__((packed))
{
	uint8_t reg_id;		// latch id
	uint8_t cascade;	// num of shift regs in cascade, most have value 3
	uint8_t reg_val[G_STORAGE_FOR_ONE_REG_BYTES]; // value saved in bytes
} reg_t;

typedef struct __attribute__((packed))
{
	uint32_t running_id;
	uint8_t ic_id;
	uint8_t command_code;
	uint8_t num_chunks; //number of chunks of sub data
	void * data[0];		// placement for a pointer, can point to  reg_t or pot_t
} mlab_data_t;

#endif /* INC_MLAB_NUCLEO_IF_H_ */


