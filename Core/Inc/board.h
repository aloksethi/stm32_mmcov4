/*
 * board.h
 *
 *  Created on: Jan 25, 2021
 *      Author: asethi
 */

#ifndef INC_BOARD_H_
#define INC_BOARD_H_

#include "FreeRTOS.h"
#include "semphr.h"

extern I2C_HandleTypeDef g_hi2c1;
extern SPI_HandleTypeDef g_hspi1;
extern UART_HandleTypeDef g_huart3;
extern SemaphoreHandle_t g_mutex_i2c_op;
extern SemaphoreHandle_t g_mutex_spi_op;

#define CHIP_CLK_DELAY				20000

#define SYNTH_POW_Pin				GPIO_PIN_7
#define SYNTH_POW_Port				GPIOC
#define EN_3V3_POW_Pin				GPIO_PIN_1
#define EN_3V3_POW_Port				GPIOB

#define LO_SW1_Pin					GPIO_PIN_10
#define LO_SW1_Port					GPIOB
#define LO_SW2_Pin					GPIO_PIN_15
#define LO_SW2_Port					GPIOB

#define EN_SUP_1_Pin 				GPIO_PIN_4
#define EN_SUP_1_Port 				GPIOB
#define EN_SUP_2_Pin 				GPIO_PIN_11
#define EN_SUP_2_Port 				GPIOC
#define EN_SUP_3_Pin 				GPIO_PIN_5
#define EN_SUP_3_Port 				GPIOF
#define EN_SUP_4_Pin 				GPIO_PIN_3
#define EN_SUP_4_Port 				GPIOA
#define EN_SUP_5_Pin 				GPIO_PIN_4
#define EN_SUP_5_Port 				GPIOF
#define EN_POW_BOARD_Pin 			GPIO_PIN_10
#define EN_POW_BOARD_Port 			GPIOC

#define LED_RED_Pin 				GPIO_PIN_14
#define LED_RED_Port 				GPIOB
#define LED_GREEN_Pin 				GPIO_PIN_0
#define LED_GREEN_Port 				GPIOB
#define LED_BLUE_Pin 				GPIO_PIN_7
#define LED_BLUE_Port 				GPIOB

#define IC_CLK_Pin					GPIO_PIN_8
#define IC_CLK_Port					GPIOC
#define IC_DATA_Pin					GPIO_PIN_8
#define IC_DATA_Port				GPIOB
#define IC1_LE_Pin					GPIO_PIN_6
#define IC1_LE_Port					GPIOC
#define IC2_LE_Pin					GPIO_PIN_12
#define IC2_LE_Port					GPIOB

#define USER_Btn_Pin 				GPIO_PIN_13
#define USER_Btn_GPIO_Port 			GPIOC
#define MCO_Pin 					GPIO_PIN_0
#define MCO_GPIO_Port 				GPIOH
#define RMII_MDC_Pin 				GPIO_PIN_1
#define RMII_MDC_GPIO_Port 			GPIOC
#define RMII_REF_CLK_Pin 			GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port 		GPIOA
#define RMII_MDIO_Pin 				GPIO_PIN_2
#define RMII_MDIO_GPIO_Port 		GPIOA
#define RMII_CRS_DV_Pin 			GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port 		GPIOA
#define RMII_RXD0_Pin 				GPIO_PIN_4
#define RMII_RXD0_GPIO_Port 		GPIOC
#define RMII_RXD1_Pin 				GPIO_PIN_5
#define RMII_RXD1_GPIO_Port 		GPIOC
#define RMII_TXD1_Pin 				GPIO_PIN_13
#define RMII_TXD1_GPIO_Port 		GPIOB
#define STLK_RX_Pin 				GPIO_PIN_8
#define STLK_RX_GPIO_Port 			GPIOD
#define STLK_TX_Pin 				GPIO_PIN_9
#define STLK_TX_GPIO_Port 			GPIOD
#define USB_PowerSwitchOn_Pin 		GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin 		GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port 	GPIOG
#define USB_SOF_Pin 				GPIO_PIN_8
#define USB_SOF_GPIO_Port 			GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG

void board_red_led_toggle(void);
void board_green_led_toggle(void);
void board_blue_led_toggle(void);

void board_synth_power_on(void);
void board_synth_power_off(void);
void board_3v3_power_on(void);
void board_3v3_power_off(void);

void board_set_lo_switch(uint8_t);

void board_pb_sup1_en(void);
void board_pb_sup1_dis(void);
void board_pb_sup2_en(void);
void board_pb_sup2_dis(void);
void board_pb_sup3_en(void);
void board_pb_sup3_dis(void);
void board_pb_sup4_en(void);
void board_pb_sup4_dis(void);
void board_pb_sup5_en(void);
void board_pb_sup5_dis(void);
void board_pb_lcl5v_en(void);
void board_pb_lcl5v_dis(void);

void board_init(void);

#endif /* INC_BOARD_H_ */
