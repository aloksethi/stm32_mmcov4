/*
 * heartbeat.h
 *
 *  Created on: Jan 25, 2021
 *      Author: asethi
 */

#ifndef INC_HEARTBEAT_H_
#define INC_HEARTBEAT_H_

#define LED_FLASH_RATE_BASE	( ( TickType_t ) 1000 )


void vStartLEDFlashTasks( UBaseType_t uxPriority );


#endif /* INC_HEARTBEAT_H_ */
