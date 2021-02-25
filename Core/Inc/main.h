/*
 * main.h
 *
 *  modified on: Jan 25, 2021
 *      Author: asethi
 *
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f4xx_hal.h"

#include "board.h"
#include "trace.h"
#include "heartbeat.h"
#include "mlab_handler.h"
#include "prog_chip_reg.h"
#include "mlab_nucleo_if.h"
#include "pot_handler.h"

#define CHIP_REG_HANDLER_TASK_PRIORITY		(configMAX_PRIORITIES - 1UL)  //--> programming chip, so when on  should be top one
#define SYNTH_TASK_PRIORITY					(tskIDLE_PRIORITY + 3UL)
#define POT_TASK_PRIORITY					(tskIDLE_PRIORITY + 3UL)
#define MATLAB_HANLDER_TASK_PRIORITY		(tskIDLE_PRIORITY + 2UL)  //-->talking with matlab
#define SENSOR_TASK_PRIORITY				(tskIDLE_PRIORITY + 1UL)
#define LED_FLASH_TASK_PRIORITY				(tskIDLE_PRIORITY + 0UL)

#define CHIP_REG_STACK_SIZE					(2 * configMINIMAL_STACK_SIZE)
#define SYNTH_STACK_SIZE					(3 * configMINIMAL_STACK_SIZE)
#define POT_STACK_SIZE						(3 * configMINIMAL_STACK_SIZE)
#define MATLAB_HANLDER_STACK_SIZE			(3 * configMINIMAL_STACK_SIZE)
#define SENSOR_STACK_SIZE					(3 * configMINIMAL_STACK_SIZE)
#define LED_STACK_SIZE						(1 * configMINIMAL_STACK_SIZE)

extern QueueHandle_t g_pc_queue_handle;

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

