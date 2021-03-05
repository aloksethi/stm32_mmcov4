/*
 * pot_handler.h
 *
 *  Created on: 25.2.2021
 *      Author: asethi
 */

#ifndef INC_POT_HANDLER_H_
#define INC_POT_HANDLER_H_

#define POT_QUEUE_LENGTH	5

#define I2C_POT_SUP1_ADDRESS		(0x58 | (0x0 <<1))
#define I2C_POT_SUP2_ADDRESS		(0x58 | (0x2 <<1))
#define I2C_POT_SUP3_ADDRESS		(0x58 | (0x2 <<1))
#define I2C_POT_SUP4_ADDRESS		(0x58 | (0x1 <<1))
#define I2C_POT_SUP5_ADDRESS		(0x58 | (0x0 <<1))

#define I2C_POT_SUP1_CHANNEL		0
#define I2C_POT_SUP2_CHANNEL		1
#define I2C_POT_SUP3_CHANNEL		0
#define I2C_POT_SUP4_CHANNEL		1
#define I2C_POT_SUP5_CHANNEL		1

typedef struct __attribute__((packed))
{
	uint8_t sup_id;
	uint8_t sup_status;
	uint8_t pot_val;
} pot_data_t;

typedef struct __attribute__((packed))
{
	uint8_t command_code;
	pot_data_t pot_data;
} pot_queue_data_t;

extern QueueHandle_t g_pot_queue_handle;

#endif /* INC_POT_HANDLER_H_ */
