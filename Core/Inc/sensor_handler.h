/*
 * sensor_handler.h
 *
 *  Created on: 2.3.2021
 *      Author: asethi
 */

#ifndef INC_SENSOR_HANDLER_H_
#define INC_SENSOR_HANDLER_H_

extern TaskHandle_t g_handle_sensor_task;
extern QueueHandle_t g_sensor_queue_handle;

/*
 * U2 meas current, v1-2 ->sup1, v3-4->sup2, v5-6->sup3, v7-8->sup4
 * U3 meas V, v1-> sup1, v2->sup2, v3->sup3, v4->sup4, rest not used (bias current, temp)
 * U4 v1-2->sup5 current, v3->sup5 voltage, rest not used (negative supply, temp)
 */
#define LTC2991_I2C_ADDRESS_U2 				0x9A
#define LTC2991_I2C_ADDRESS_U3 				0x98
#define LTC2991_I2C_ADDRESS_U4		 		0x9E
#define LTC2991_I2C_GLOBAL_ADDRESS 0x77  //  Global Address

#define SENSOR_TEMP_SCALE       100 //temp is scaled by 100
#define SENSOR_VOLT_SCALE       1000 //voltage is scaled by 1000
#define SENSOR_CURR_SCALE       1000000 //current is scaled by 1e6

#define LTC2991_STATUS_LOW_REG              0x00    //!< Data_Valid Bits(V1 Through V8)
#define LTC2991_STATUS_HIGH_REG             0x01    //!< Data_valid bits
#define LTC2991_CHANNEL_ENABLE_REG          0x01    //!< Channel Enable, Vcc, T_internal Conversion Status, Trigger
#define LTC2991_CONTROL_V1234_REG           0x06    //!< V1, V2, V3, and V4 Control Register
#define LTC2991_CONTROL_V5678_REG           0x07    //!< V5, V6, V7, AND V8 Control Register
#define LTC2991_CONTROL_PWM_Tinternal_REG   0x08    //!< PWM Threshold and T_internal Control Register
#define LTC2991_PWM_THRESHOLD_MSB_REG       0x09    //!< PWM Threshold
#define LTC2991_V1_MSB_REG                  0x0A    //!< V1, or T_R1 T MSB
#define LTC2991_V1_LSB_REG                  0x0B    //!< V1, or T_R1 T LSB
#define LTC2991_V2_MSB_REG                  0x0C    //!< V2, V1-V2, or T_R2 Voltage MSB
#define LTC2991_V2_LSB_REG                  0x0D    //!< V2, V1-V2, or T_R2 Voltage LSB
#define LTC2991_V3_MSB_REG                  0x0E    //!< V3, or T_R2 T MSB
#define LTC2991_V3_LSB_REG                  0x0F    //!< V3, or T_R2 T LSB
#define LTC2991_V4_MSB_REG                  0x10    //!< V4, V3-V4, or T_R2 Voltage MSB
#define LTC2991_V4_LSB_REG                  0x11    //!< V4, V3-V4, or T_R2 Voltage LSB
#define LTC2991_V5_MSB_REG                  0x12    //!< V5, or T_R3 T MSB
#define LTC2991_V5_LSB_REG                  0x13    //!< V5, or T_R3 T LSB
#define LTC2991_V6_MSB_REG                  0x14    //!< V6, V5-V6, or T_R3 Voltage MSB
#define LTC2991_V6_LSB_REG                  0x15    //!< V6, V5-V6, or T_R3 Voltage LSB
#define LTC2991_V7_MSB_REG                  0x16    //!< V7, or T_R4 T MSB
#define LTC2991_V7_LSB_REG                  0x17    //!< V7, or T_R4 T LSB
#define LTC2991_V8_MSB_REG                  0x18    //!< V8, V7-V8, or T_R4 Voltage MSB
#define LTC2991_V8_LSB_REG                  0x19    //!< V8, V7-V8, or T_R4 Voltage LSB
#define LTC2991_T_Internal_MSB_REG          0x1A    //!< T_Internal MSB
#define LTC2991_T_Internal_LSB_REG          0x1B    //!< T_Internal LSB
#define LTC2991_Vcc_MSB_REG                 0x1C    //!< Vcc MSB
#define LTC2991_Vcc_LSB_REG                 0x1D    //!< Vcc LSB

//#define LTC2991_V78_READY					0x80
//#define LTC2991_V56_READY					0x40
//#define LTC2991_V34_READY					0x20
//#define LTC2991_V12_READY					0x10

#define LTC2991_V78_READY					0x40
#define LTC2991_V56_READY					0x10
#define LTC2991_V34_READY					0x04
#define LTC2991_V12_READY					0x01

#define LTC2991_V1_READY					0x01
#define LTC2991_V2_READY					0x02
#define LTC2991_V3_READY					0x04
#define LTC2991_V4_READY					0x08

#define LTC2991_TIN_READY					0x02
#define LTC2991_VCC_READY					0x01

/*! @} */
/*! @name LTC2991_CHANNEL_ENABLE_REG SETTINGS
 Bitwise OR settings, and write to LTC2991_CHANNEL_ENABLE_REG to configure settings.
 Bitwise AND with value read from LTC2991_CHANNEL_ENABLE_REG to determine present setting.
 @{ */

#define LTC2991_V7_V8_TR4_ENABLE              0x80  //!< Enable V7-V8 measurements, including TR4 temperature
#define LTC2991_V5_V6_TR3_ENABLE              0x40  //!< Enable V5-V6 measurements, including TR3 temperature
#define LTC2991_V3_V4_TR2_ENABLE              0x20  //!< Enable V3-V4 measurements, including TR2 temperature
#define LTC2991_V1_V2_TR1_ENABLE              0x10  //!< Enable V1-V2 measurements, including TR1 temperature
#define LTC2991_VCC_TINTERNAL_ENABLE          0x08  //!< Enable Vcc internal voltage measurement
#define LTC2991_ENABLE_ALL_CHANNELS           0xF8  //!< Use to enable all LTC2991 channels.  Equivalent to bitwise OR'ing all channel enables.
#define LTC2991_BUSY                          0x04  //!< LTC2991 Busy Bit

/*! @} */
/*! @name LTC2991_CONTROL_V1234_REG SETTINGS
 Bitwise OR settings, and write to LTC2991_CONTROL_V1234_REG to configure settings.
 Bitwise AND with value read from LTC2991_CONTROL_V1234_REG to determine present setting.
 @{ */

#define LTC2991_V3_V4_FILTER_ENABLE           0x80 //!< Enable filters on V3-V4
#define LTC2991_V3_V4_KELVIN_ENABLE           0x40 //!< Enable V3-V4 for Kelvin. Otherwise, Celsius.
#define LTC2991_V3_V4_TEMP_ENABLE             0x20 //!< Enable V3-V4 temperature mode.
#define LTC2991_V3_V4_DIFFERENTIAL_ENABLE     0x10 //!< Enable V3-V4 differential mode.  Otherwise, single-ended.
#define LTC2991_V1_V2_FILTER_ENABLE           0x08 //!< Enable filters on V1-V2
#define LTC2991_V1_V2_KELVIN_ENABLE           0x04 //!< Enable V1-V2 for Kelvin. Otherwise, Celsius.
#define LTC2991_V1_V2_TEMP_ENABLE             0x02 //!< Enable V1-V2 temperature mode.
#define LTC2991_V1_V2_DIFFERENTIAL_ENABLE     0x01 //!< Enable V1-V2 differential mode.  Otherwise, single-ended.

/*! @} */
/*! @name LTC2991_CONTROL_V5678_REG SETTINGS
 Bitwise OR settings, and write to LTC2991_CONTROL_V5678_REG to configure settings.
 Bitwise AND with value read from LTC2991_CONTROL_V5678_REG to determine present setting.
 @{ */

#define LTC2991_V7_V8_FILTER_ENABLE           0x80 //!< Enable filters on V7-V8
#define LTC2991_V7_V8_KELVIN_ENABLE           0x40 //!< Enable V7-V8 for Kelvin. Otherwise, Celsius.
#define LTC2991_V7_V8_TEMP_ENABLE             0x20 //!< Enable V7-V8 temperature mode.
#define LTC2991_V7_V8_DIFFERENTIAL_ENABLE     0x10 //!< Enable V7-V8 differential mode.  Otherwise, single-ended.
#define LTC2991_V5_V6_FILTER_ENABLE           0x08 //!< Enable filters on V5-V6
#define LTC2991_V5_V6_KELVIN_ENABLE           0x04 //!< Enable V5-V6 for Kelvin. Otherwise, Celsius.
#define LTC2991_V5_V6_TEMP_ENABLE             0x02 //!< Enable V5-V6 temperature mode.
#define LTC2991_V5_V6_DIFFERENTIAL_ENABLE     0x01 //!< Enable V5-V6 differential mode.  Otherwise, single-ended.

/*! @} */
/*! @name LTC2991_CONTROL_PWM_Tinternal_REG SETTINGS
 Bitwise OR settings, and write to LTC2991_CONTROL_PWM_Tinternal_REG to configure settings.
 Bitwise AND with value read from LTC2991_CONTROL_PWM_Tinternal_REG to determine present setting.
 @{ */

#define LTC2991_PWM_0                         0x80 //!< PWM threshold Least Significant Bit
#define LTC2991_PWM_INVERT                    0x40 //!< Invert PWM
#define LTC2991_PWM_ENABLE                    0x20 //!< Enable PWM
#define LTC2991_REPEAT_MODE                   0x10 //!< Enable Repeated Aquisition Mode
#define LTC2991_INT_FILTER_ENABLE             0x08 //!< Enable Internal Temperature Filter
#define LTC2991_INT_KELVIN_ENABLE             0x04 //!< Enable internal temperature for Kelvin. Otherwise, Celsius.
/*!@} */

typedef struct __attribute__((packed))
{
	uint32_t voltage_1v0;	//scaled by 10
	uint32_t voltage_2v0; 	// scaled by 10
	uint32_t voltage_3v0; 	// scaled by 10
	uint32_t voltage_4v0; 	// scaled by 10
	uint32_t voltage_5v0;	//scaled by 10
	uint32_t current_1v0; 	//scaled to give uA
	uint32_t current_2v0; 	//scaled to give uA
	uint32_t current_3v0; 	//scaled to give uA
	uint32_t current_4v0; 	//scaled to give uA
	uint32_t current_5v0; 	//scaled to give uA
} sensor_data_t;

#endif /* INC_SENSOR_HANDLER_H_ */
