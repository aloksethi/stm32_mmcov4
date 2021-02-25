/*
 * pot_handler.h
 *
 *  Created on: 25.2.2021
 *      Author: asethi
 */

#ifndef INC_POT_HANDLER_H_
#define INC_POT_HANDLER_H_

#define POT_QUEUE_LENGTH	5

typedef struct __attribute__((packed))
{
	uint8_t pot_id;
	uint8_t pot_status;
	uint8_t pot_val;
} pot_data_t;

typedef struct __attribute__((packed))
{
	uint8_t command_code;
	pot_data_t pot_data;
} pot_queue_data_t;

extern QueueHandle_t g_pot_queue_handle;

#endif /* INC_POT_HANDLER_H_ */
