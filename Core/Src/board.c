/*
 * board.c
 *
 *  Created on: Jan 25, 2021
 *      Author: asethi
 */

#include "main.h"

void board_red_led_toggle(void)
{
	HAL_GPIO_TogglePin(LED_RED_Port, LED_RED_Pin);
	return;
}

void board_green_led_toggle(void)
{
	HAL_GPIO_TogglePin(LED_GREEN_Port, LED_GREEN_Pin);
	return;
}
void board_blue_led_toggle(void)
{
	HAL_GPIO_TogglePin(LED_BLUE_Port, LED_BLUE_Pin);
	return;
}


void board_set_reg_data(uint8_t data)
{
//	if (data)
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
//	else
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
return;
}

void board_send_reg_clock(void)
{
//	for(int i=0;i<CHIP_CLK_DELAY;i++);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
//	for(int i=0;i<CHIP_CLK_DELAY;i++);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	//	GPIOA->BSRR = GPIO_PIN_7;
	//	GPIOA->BSRR = (uint32_t)GPIO_PIN_7 << 16U;
return;
}

void board_set_le(void)
{
//	for(int i=0;i<CHIP_CLK_DELAY;i++);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
//	for(int i=0;i<CHIP_CLK_DELAY;i++);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
return;
}
