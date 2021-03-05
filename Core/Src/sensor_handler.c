/*
 * sensor_handler.c
 *
 *  Created on: 24.2.2021
 *      Author: asethi
 */

#include "main.h"
#include "sensor_handler.h"

static const float LTC2991_SINGLE_ENDED_lsb = 3.05176E-04f;
//! Typical differential LSB weight in volts
static const float LTC2991_DIFFERENTIAL_lsb = 1.90735E-05f;
//! Typical VCC LSB weight in volts
static const float LTC2991_VCC_lsb = 3.05176E-04f;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
static const float LTC2991_TEMPERATURE_lsb = 0.0625f;

static TaskHandle_t g_handle_sensor_task;
sensor_data_t g_sensor_data;

static uint8_t i2c_sensor_read_reg(uint16_t addr, uint8_t reg, uint8_t *data)
{
	HAL_StatusTypeDef ret1, ret2;
	uint8_t ret = 0;

	if (xSemaphoreTake(g_mutex_i2c_op, portMAX_DELAY) == pdTRUE)
	{
		ret1 = HAL_I2C_Master_Transmit(&g_hi2c1, addr, &reg, 1, 100);
		ret2 = HAL_I2C_Master_Receive(&g_hi2c1, addr, data, 1, 100);
		xSemaphoreGive(g_mutex_i2c_op);
	}

	if ((HAL_OK != ret1) || (HAL_OK != ret2))
	{
		trace_printf("failed to read sensor %x:%x \n", ret1, ret2);
		ret = -1;
	}
	return ret;
}

static uint8_t i2c_sensor_set_reg(uint16_t addr, uint8_t *data, uint8_t size)
{
	HAL_StatusTypeDef ret1;
	uint8_t ret = 0;
	if (xSemaphoreTake(g_mutex_i2c_op, portMAX_DELAY) == pdTRUE)
	{
		ret1 = HAL_I2C_Master_Transmit(&g_hi2c1, addr, data, size, 10);
		xSemaphoreGive(g_mutex_i2c_op);
	}
	if (HAL_OK != ret1)
	{
		trace_printf("failed to set sensor %x \n", ret1);
		ret = -1;
	}
	return ret;
}

static uint8_t i2c_sensor_config_u2(void)
{
	uint8_t ret;
	uint8_t data[2] =
	{ 0, 0 };

	data[0] = LTC2991_CONTROL_V1234_REG;
	data[1] = LTC2991_V1_V2_DIFFERENTIAL_ENABLE
			| LTC2991_V3_V4_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_FILTER_ENABLE
			| LTC2991_V3_V4_FILTER_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U2, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR U2 CONFIG V1234 FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_V5678_REG;
	data[1] = LTC2991_V5_V6_DIFFERENTIAL_ENABLE
			| LTC2991_V7_V8_DIFFERENTIAL_ENABLE | LTC2991_V5_V6_FILTER_ENABLE
			| LTC2991_V7_V8_FILTER_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U2, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR U2 CONFIG V5678 FAILED : %d\n", ret);
		return ret;
	}

	// enable all measurements
	data[0] = LTC2991_CHANNEL_ENABLE_REG;
	data[1] = LTC2991_ENABLE_ALL_CHANNELS;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U2, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR CONFIG U2 CH_EN FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_PWM_Tinternal_REG;
	data[1] = LTC2991_REPEAT_MODE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U2, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR CONFIG repeat mode FAILED : %d\n", ret);
		return ret;
	}
	return ret;
}

static uint8_t i2c_sensor_config_u3(void)
{
	uint8_t ret;
	uint8_t data[2] =
	{ 0, 0 };

	data[0] = LTC2991_CONTROL_V1234_REG;
	data[1] = LTC2991_V1_V2_FILTER_ENABLE | LTC2991_V3_V4_FILTER_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U3, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR U3 CONFIG V1234 FAILED : %d\n", ret);
		return ret;
	}

	// enable all measurements
	data[0] = LTC2991_CHANNEL_ENABLE_REG;
	data[1] = LTC2991_V1_V2_TR1_ENABLE | LTC2991_V3_V4_TR2_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U3, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR CONFIG U3 CH_EN FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_PWM_Tinternal_REG;
	data[1] = LTC2991_REPEAT_MODE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U3, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR CONFIG repeat mode FAILED : %d\n", ret);
		return ret;
	}
	return ret;
}

static uint8_t i2c_sensor_config_u4(void)
{
	uint8_t ret;
	uint8_t data[2] =
	{ 0, 0 };

	data[0] = LTC2991_CONTROL_V1234_REG;
	data[1] = LTC2991_V1_V2_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_FILTER_ENABLE
			| LTC2991_V3_V4_FILTER_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U4, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR U4 CONFIG V1234 FAILED : %d\n", ret);
		return ret;
	}

	// enable all measurements
	data[0] = LTC2991_CHANNEL_ENABLE_REG;
	data[1] = LTC2991_V1_V2_TR1_ENABLE | LTC2991_V3_V4_TR2_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U4, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR CONFIG U4 CH_EN FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_PWM_Tinternal_REG;
	data[1] = LTC2991_REPEAT_MODE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U4, &data[0], 2);
	if (ret)
	{
		trace_printf("I2C SENSOR CONFIG repeat mode FAILED : %d\n", ret);
		return ret;
	}
	return ret;
}

static void i2c_sensor_config(void)
{
	i2c_sensor_config_u2();
	i2c_sensor_config_u3();
	i2c_sensor_config_u4();
}

static HAL_StatusTypeDef i2c_sensor_meas_current(uint16_t addr, uint8_t msb_reg,
		int32_t *op)
{
	HAL_StatusTypeDef ret1, ret2;
	uint8_t msb, lsb;
	uint16_t code;
	float current;
	int16_t sign;

	ret1 = i2c_sensor_read_reg(addr, (uint8_t) (msb_reg + 1), &lsb);
	ret2 = i2c_sensor_read_reg(addr, msb_reg, &msb);

	if ((HAL_OK != ret1) || (HAL_OK != ret2))
		return HAL_ERROR;

	code = (uint16_t) (lsb | (msb << 8));

	if (code & (0x1 << 14))
	{
		code = (uint16_t) ((code ^ 0x7FFF) + 1); //! 1)Converts two's complement to binary
		sign = -1;
	}
	else
	{
		code = code & 0x3fff;
		sign = 1;
	}
	//resistor is assumed to be 0.1, so instead of dividing, multiplying by 10
	current = (float) (code * LTC2991_DIFFERENTIAL_lsb * sign
			* SENSOR_CURR_SCALE * 10);
	if (-1 == sign)
		*op = (int32_t) (current - 0.5);
	else
		*op = (int32_t) (current + 0.5);

	return ret1;
}

static uint8_t i2c_sensor_meas_voltage(uint16_t addr, uint8_t msb_reg,
		int32_t *op)
{
	HAL_StatusTypeDef ret1, ret2;
	uint8_t msb, lsb;
	uint16_t code;
	float voltage;
	int16_t sign;

	ret1 = i2c_sensor_read_reg(addr, (uint8_t) (msb_reg + 1), &lsb);
	ret2 = i2c_sensor_read_reg(addr, msb_reg, &msb);

	if ((HAL_OK != ret1) || (HAL_OK != ret2))
		return -1;

	trace_printf("msb: %d, lsb: %d\n", msb, lsb);
	code = (uint16_t) (lsb | ((uint16_t) msb << 8));

	if (code & (0x1 << 14))
	{
		code = (uint16_t) ((code ^ 0x7FFF) + 1); //! 1)Converts two's complement to binary
		sign = -1;
	}
	else
	{
		code = code & 0x3fff;
		sign = 1;
	}
	trace_printf("sign = %d, code = %d \n", sign, code);
	voltage = (float) (code * LTC2991_SINGLE_ENDED_lsb * sign
			* SENSOR_VOLT_SCALE); //! 2) Convert code to voltage form differential lsb

	if (-1 == sign)
		*op = (int) (voltage - 0.5);
	else
		*op = (int) (voltage + 0.5);

	return 0;
}

static uint8_t i2c_sensor_u2_read(sensor_data_t *op)
{
	uint8_t status_h;
	uint8_t ret1, ret = -1;
	int32_t meas;

	ret1 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_U2, LTC2991_STATUS_HIGH_REG,
			&status_h);

	if (ret1)
		goto SENSOR_U2_ERROR;

	if (status_h & LTC2991_V12_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V2_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U2_ERROR;
		op->current_1v0 = meas;
	}

	if (status_h & LTC2991_V34_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V4_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U2_ERROR;
		op->current_2v0 = meas;
	}

	if (status_h & LTC2991_V56_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V6_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U2_ERROR;
		op->current_3v0 = meas;
	}

	if (status_h & LTC2991_V78_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V8_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U2_ERROR;
		op->current_4v0 = meas;
	}

	ret = 0;
	SENSOR_U2_ERROR: return ret;
}

static HAL_StatusTypeDef i2c_sensor_u3_read(sensor_data_t *op)
{
	uint8_t status_l;
	uint8_t ret1, ret = -1;
	int32_t meas;

	ret1 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_U3, LTC2991_STATUS_LOW_REG,
			&status_l);

	if (ret1)
		goto SENSOR_U3_ERROR;

	if (status_l & LTC2991_V1_READY)
	{
		ret1 = i2c_sensor_meas_voltage(LTC2991_I2C_ADDRESS_U3,
		LTC2991_V1_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U3_ERROR;
		op->voltage_1v0 = meas;
	}

	if (status_l & LTC2991_V2_READY)
	{
		ret1 = i2c_sensor_meas_voltage(LTC2991_I2C_ADDRESS_U3,
		LTC2991_V2_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U3_ERROR;
		op->voltage_2v0 = meas;
	}

	if (status_l & LTC2991_V3_READY)
	{
		ret1 = i2c_sensor_meas_voltage(LTC2991_I2C_ADDRESS_U3,
		LTC2991_V3_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U3_ERROR;
		op->voltage_3v0 = meas;
	}

	if (status_l & LTC2991_V4_READY)
	{
		ret1 = i2c_sensor_meas_voltage(LTC2991_I2C_ADDRESS_U3,
		LTC2991_V4_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U3_ERROR;
		op->voltage_4v0 = meas;
	}

	ret = 0;
	SENSOR_U3_ERROR: return ret;
}

static uint8_t i2c_sensor_u4_read(sensor_data_t *op)
{
	uint8_t status_l, status_h;
	uint8_t ret1, ret2, ret = -1;
	int32_t meas;

	ret1 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_U4, LTC2991_STATUS_LOW_REG,
			&status_l);
	ret2 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_U4, LTC2991_STATUS_HIGH_REG,
			&status_h);

	if (ret1 || ret2)
		goto SENSOR_U4_ERROR;

	if (status_h & LTC2991_V12_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U4,
		LTC2991_V2_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U4_ERROR;
		op->current_5v0 = meas;
	}

	if (status_l & LTC2991_V3_READY)
	{
		ret1 = i2c_sensor_meas_voltage(LTC2991_I2C_ADDRESS_U4,
		LTC2991_V3_MSB_REG, &meas);
		if (ret1)
			goto SENSOR_U4_ERROR;
		op->voltage_5v0 = (2 * meas);
	}

//trace_printf("5vis:%d\n",op->voltage_5v0);

	ret = 0;
	SENSOR_U4_ERROR: return ret;
}

void vSensorHandlerTask(void *pvParameters)
{
	while (1)
	{
		uint8_t ret1 = 0, ret2 = 0, ret3 = 0;

		vTaskDelay(1000);
		//	ret1 = i2c_sensor_u2_read(&g_sensor_data);
		ret2 = i2c_sensor_u3_read(&g_sensor_data);
		ret3 = i2c_sensor_u4_read(&g_sensor_data);

		if (ret1 || ret2 || ret3)
		{
			i2c_sensor_config();
		}
		else
		{
			trace_printf(
					"Voltages: S1:%05d, S2:%05d, S3:%05d, S4:%05d, S5:%05d\n",
					g_sensor_data.voltage_1v0, g_sensor_data.voltage_2v0,
					g_sensor_data.voltage_3v0, g_sensor_data.voltage_4v0,
					g_sensor_data.voltage_5v0);
		}
	}
	return;
}

void vStartSensorTask(UBaseType_t uxPriority)
{
	BaseType_t xReturned;

	i2c_sensor_config();

	/* Spawn the task. */
//	xReturned = xTaskCreate(vSensorHandlerTask, "SENSOR", SENSOR_STACK_SIZE,
//	NULL, uxPriority, (TaskHandle_t*) &g_handle_sensor_task);
//	if (xReturned != pdPASS)
//	{
//		trace_printf("failed to create sensor handling task\n");
//		Error_Handler();
//	}
	return;
}
