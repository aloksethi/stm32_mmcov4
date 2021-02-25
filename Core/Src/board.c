/*
 * board.c
 *
 *  Created on: Jan 25, 2021
 *      Author: asethi
 */

#include "main.h"

I2C_HandleTypeDef g_hi2c1;
SPI_HandleTypeDef g_hspi1;
UART_HandleTypeDef g_huart3;

static void board_i2c_init(void)
{
	g_hi2c1.Instance = I2C1;
	g_hi2c1.Init.ClockSpeed = 100000;
	g_hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	g_hi2c1.Init.OwnAddress1 = 0;
	g_hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	g_hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	g_hi2c1.Init.OwnAddress2 = 0;
	g_hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	g_hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&g_hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&g_hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK)
	{
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&g_hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

static void board_spi_init(void)
{
	g_hspi1.Instance = SPI1;
	g_hspi1.Init.Mode = SPI_MODE_MASTER;
	g_hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	g_hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	g_hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	g_hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	g_hspi1.Init.NSS = SPI_NSS_SOFT;
	g_hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	g_hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	g_hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	g_hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	g_hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&g_hspi1) != HAL_OK)
	{
		Error_Handler();
	}
	return;
}

static void board_uart_init(void)
{
	g_huart3.Instance = USART3;
	g_huart3.Init.BaudRate = 115200;
	g_huart3.Init.WordLength = UART_WORDLENGTH_8B;
	g_huart3.Init.StopBits = UART_STOPBITS_1;
	g_huart3.Init.Parity = UART_PARITY_NONE;
	g_huart3.Init.Mode = UART_MODE_TX_RX;
	g_huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	g_huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&g_huart3) != HAL_OK)
	{
		Error_Handler();
	}
	return;
}

//static void board_wdog_init(void)
//{
//  hwwdg.Instance = WWDG;
//  hwwdg.Init.Prescaler = WWDG_PRESCALER_1;
//  hwwdg.Init.Window = 64;
//  hwwdg.Init.Counter = 64;
//  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
//  if (HAL_WWDG_Init(&hwwdg) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  return;
//}

static void board_en_pins_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
	{ 0 };

	GPIO_InitStruct.Pin = EN_SUP_1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_SUP_1_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = EN_SUP_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_SUP_2_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = EN_SUP_3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_SUP_3_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = EN_SUP_4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_SUP_4_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = EN_SUP_5_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_SUP_5_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = EN_POW_BOARD_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_POW_BOARD_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SYNTH_POW_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SYNTH_POW_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = EN_3V3_POW_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_3V3_POW_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LO_SW1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LO_SW1_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LO_SW2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LO_SW2_Port, &GPIO_InitStruct);

	// power board enable pins are active low
	HAL_GPIO_WritePin(EN_SUP_1_Port, EN_SUP_1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_SUP_2_Port, EN_SUP_2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_SUP_3_Port, EN_SUP_3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_SUP_4_Port, EN_SUP_4_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_SUP_5_Port, EN_SUP_5_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN_POW_BOARD_Port, EN_POW_BOARD_Pin, GPIO_PIN_RESET); // without this there is no control

	HAL_GPIO_WritePin(SYNTH_POW_Port, SYNTH_POW_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_3V3_POW_Port, EN_3V3_POW_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LO_SW1_Port, LO_SW1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LO_SW2_Port, LO_SW2_Pin, GPIO_PIN_RESET);
}

static void board_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
	{ 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(
	GPIOB,
			GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | LED_RED_Pin
					| GPIO_PIN_15 | GPIO_PIN_4 | LED_BLUE_Pin | LED_GREEN_Pin
					| GPIO_PIN_8, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(
	GPIOC, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_11,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : PE2 PE3 PE4 PE5
	 PE6 PE7 PE8 PE9
	 PE10 PE11 PE12 PE13
	 PE14 PE15 PE0 PE1 */
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5
			| GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10
			| GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
			| GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PF0 PF1 PF2 PF3
	 PF6 PF7 PF8 PF9
	 PF10 PF11 PF12 PF13
	 PF14 PF15 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
			| GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10
			| GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
			| GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : PF4 PF5 */
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : PC0 PC2 PC3 PC12 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA0 PA4 PA15 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_4 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB2 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB1 PB10 PB11 PB12
	 LD3_Pin PB15 PB4 LED_BLUE_Pin
	 PB8 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
			| LED_RED_Pin | GPIO_PIN_15 | GPIO_PIN_4 | LED_BLUE_Pin
			| LED_GREEN_Pin | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PG0 PG1 PG3 PG4
	 PG5 PG8 PG9 PG10
	 PG12 PG14 PG15 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4
			| GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_12
			| GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pins : PD10 PD11 PD12 PD13
	 PD14 PD15 PD0 PD1
	 PD2 PD3 PD4 PD5
	 PD6 PD7 */
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
			| GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2
			| GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : PG2 */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PC6 PC7 PC8 LD1_Pin
	 PC10 PC11 */
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10
			| GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
	GPIO_InitStruct.Pin = USB_SOF_Pin | USB_ID_Pin | USB_DM_Pin | USB_DP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_VBUS_Pin */
	GPIO_InitStruct.Pin = USB_VBUS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

}

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
void board_synth_power_on(void)
{
	HAL_GPIO_WritePin(SYNTH_POW_Port, SYNTH_POW_Pin, GPIO_PIN_SET);

	return;
}
void board_synth_power_off(void)
{
	HAL_GPIO_WritePin(SYNTH_POW_Port, SYNTH_POW_Pin, GPIO_PIN_RESET);

	return;
}
void board_3v3_power_on(void)
{
	HAL_GPIO_WritePin(EN_3V3_POW_Port, EN_3V3_POW_Pin, GPIO_PIN_SET);

	return;
}
void board_3v3_power_off(void)
{
	HAL_GPIO_WritePin(EN_3V3_POW_Port, EN_3V3_POW_Pin, GPIO_PIN_RESET);

	return;
}
void board_set_lo_switch(uint8_t val)
{
	switch (val)
	{
	case 0:
	{
		HAL_GPIO_WritePin(LO_SW1_Port, LO_SW1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LO_SW2_Port, LO_SW2_Pin, GPIO_PIN_RESET);
		break;
	}
	case 1:
	{
		HAL_GPIO_WritePin(LO_SW1_Port, LO_SW1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LO_SW2_Port, LO_SW2_Pin, GPIO_PIN_RESET);
		break;
	}
	case 2:
	{
		HAL_GPIO_WritePin(LO_SW1_Port, LO_SW1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LO_SW2_Port, LO_SW2_Pin, GPIO_PIN_SET);
		break;
	}
	}
	return;
}
void board_pb_sup1_en(void)
{
	HAL_GPIO_WritePin(EN_SUP_1_Port, EN_SUP_1_Pin, GPIO_PIN_RESET);
	return;
}
void board_pb_sup1_dis(void)
{
	HAL_GPIO_WritePin(EN_SUP_1_Port, EN_SUP_1_Pin, GPIO_PIN_SET);
	return;
}
void board_pb_sup2_en(void)
{
	HAL_GPIO_WritePin(EN_SUP_2_Port, EN_SUP_2_Pin, GPIO_PIN_RESET);
	return;
}
void board_pb_sup2_dis(void)
{
	HAL_GPIO_WritePin(EN_SUP_2_Port, EN_SUP_2_Pin, GPIO_PIN_SET);
	return;
}
void board_pb_sup3_en(void)
{
	HAL_GPIO_WritePin(EN_SUP_3_Port, EN_SUP_3_Pin, GPIO_PIN_RESET);
	return;
}
void board_pb_sup3_dis(void)
{
	HAL_GPIO_WritePin(EN_SUP_3_Port, EN_SUP_3_Pin, GPIO_PIN_SET);
	return;
}
void board_pb_sup4_en(void)
{
	HAL_GPIO_WritePin(EN_SUP_4_Port, EN_SUP_4_Pin, GPIO_PIN_RESET);
	return;
}
void board_pb_sup4_dis(void)
{
	HAL_GPIO_WritePin(EN_SUP_4_Port, EN_SUP_4_Pin, GPIO_PIN_SET);
	return;
}
void board_pb_sup5_en(void)
{
	HAL_GPIO_WritePin(EN_SUP_5_Port, EN_SUP_5_Pin, GPIO_PIN_RESET);
	return;
}
void board_pb_sup5_dis(void)
{
	HAL_GPIO_WritePin(EN_SUP_5_Port, EN_SUP_5_Pin, GPIO_PIN_SET);
	return;
}
void board_pb_lcl5v_en(void)
{
	HAL_GPIO_WritePin(EN_POW_BOARD_Port, EN_POW_BOARD_Pin, GPIO_PIN_RESET);
	return;
}
void board_pb_lcl5v_dis(void)
{
	HAL_GPIO_WritePin(EN_POW_BOARD_Port, EN_POW_BOARD_Pin, GPIO_PIN_SET);
	return;
}

void board_init(void)
{
	board_gpio_init();
	board_en_pins_init();
	board_i2c_init();
	board_spi_init();
//	board_uart_init();
//	board_wdog_init();

	return;
}

