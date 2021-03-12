/*
 * sensor_handler.c
 *
 *  Created on: 24.2.2021
 *      Author: asethi
 */

#include "main.h"
#include "sensor_handler.h"

//static const float LTC2991_SINGLE_ENDED_lsb = 3.05176E-04f;
//! Typical differential LSB weight in volts
static const float LTC2991_DIFFERENTIAL_lsb = 1.90735E-05f;
//! Typical VCC LSB weight in volts
static const float LTC2991_VCC_lsb = 3.05176E-04f;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
//static const float LTC2991_TEMPERATURE_lsb = 0.0625f;

static const uint32_t LTC2991_SE_LSB_MUL = 2500; // se voltage measuremetns are in mV
static const uint32_t LTC2991_DIFF_LSB_MUL = 25000; // diff current measuremetns are in uA. sense resistor is 0.1 ohm
static const uint8_t LTC2991_SE_LSB_RHSIFT = 13;
static const uint8_t LTC2991_DIFF_LSB_RHSIFT = 17;

TaskHandle_t g_handle_sensor_task;
sensor_data_t g_sensor_data;

static uint8_t i2c_sensor_read_reg_nomutex(uint16_t addr, uint8_t reg,
		uint8_t *data)
{
	HAL_StatusTypeDef ret1, ret2;
	uint8_t ret = 0;

	ret1 = HAL_I2C_Master_Transmit(&g_hi2c1, addr, &reg, 1, 100);
	ret2 = HAL_I2C_Master_Receive(&g_hi2c1, addr, data, 1, 100);

	if ((HAL_OK != ret1) || (HAL_OK != ret2))
	{
		trace_printf("failed to read sensor %x:%x \n", ret1, ret2);
		ret = -1;
	}
	return ret;
}

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

static void code_to_voltage(uint8_t msb, uint8_t lsb, int32_t *val)
{
	uint32_t voltage;
	int8_t sign;
	uint16_t code;

	code = (uint16_t) (lsb | ((uint16_t) msb << 8));

	if (code & (0x1 << 14))
	{
		code = (uint16_t) ((code ^ 0xFFFF) + 1); //! 1)Converts two's complement to binary
		sign = -1;
	}
	else
	{
		code = code & 0x3fff;
		sign = 1;
	}
	//trace_printf("V: sign = %d, code = %d, lsb = %d, msb = %d\n", sign, code,
	//		lsb, msb);
	voltage = ((uint32_t) code * LTC2991_SE_LSB_MUL) >> LTC2991_SE_LSB_RHSIFT;

	*val = (uint32_t) (voltage & 0xffffffff) * sign;

	return;
}

static void code_to_current(uint8_t msb, uint8_t lsb, int32_t *val)
{
	uint64_t current;
	int8_t sign;
	uint16_t code;

	code = (uint16_t) (lsb) | ((uint16_t) msb << 8);

	if (code & (0x1 << 14))
	{
		code = (uint16_t) ((code ^ 0xFFFF) + 1); //! 1)Converts two's complement to binary
		sign = -1;
	}
	else
	{
		code = code & 0x3fff;
		sign = 1;
	}
	trace_printf("I sign = %d, code = %d, lsb = %d, msb = %d\n", sign, code,
			lsb, msb);

	current = (uint64_t) code * LTC2991_DIFF_LSB_MUL;
	current = current >> LTC2991_DIFF_LSB_RHSIFT;

	*val = (uint32_t) (current & 0xffffffff) * sign;

	return;
}

static void i2c_sensor_u2_read_nomutex(sensor_data_t *op)
{
	uint8_t status_h;
	uint8_t ret1, ret = 0;
	int32_t meas;
	uint8_t v2_msb, v2_lsb, v4_msb, v4_lsb, v6_msb, v6_lsb, v8_msb, v8_lsb;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_STATUS_HIGH_REG, &status_h);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V2_MSB_REG, &v2_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V2_LSB_REG, &v2_lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V4_MSB_REG, &v4_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V4_LSB_REG, &v4_lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V6_MSB_REG, &v6_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V6_LSB_REG, &v6_lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V8_MSB_REG, &v8_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_V8_LSB_REG, &v8_lsb);
	ret = ret | ret1;

	if (ret)
		goto SENSOR_U2_ERROR;

	if (status_h & LTC2991_V12_READY)
	{
		code_to_current(v2_msb, v2_lsb, &meas);
		op->current_1v0 = meas;
	}

	if (status_h & LTC2991_V34_READY)
	{
		code_to_current(v4_msb, v4_lsb, &meas);

		op->current_2v0 = meas;
	}

	if (status_h & LTC2991_V56_READY)
	{
		code_to_current(v6_msb, v6_lsb, &meas);

		op->current_3v0 = meas;
	}

	if (status_h & LTC2991_V78_READY)
	{
		code_to_current(v8_msb, v8_lsb, &meas);

		op->current_4v0 = meas;
	}

	SENSOR_U2_ERROR: return;
}

static void i2c_sensor_u3_read_nomutex(sensor_data_t *op)
{
	uint8_t status_l;
	uint8_t ret1, ret = 0;
	int32_t meas;
	uint8_t v1_msb, v1_lsb, v2_msb, v2_lsb, v3_msb, v3_lsb, v4_msb, v4_lsb;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_STATUS_LOW_REG, &status_l);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V1_MSB_REG, &v1_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V1_LSB_REG, &v1_lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V2_MSB_REG, &v2_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V2_LSB_REG, &v2_lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V3_MSB_REG, &v3_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V3_LSB_REG, &v3_lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V4_MSB_REG, &v4_msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U3,
	LTC2991_V4_LSB_REG, &v4_lsb);
	ret = ret | ret1;

	if (ret)
		goto SENSOR_U3_ERROR;

	if (status_l & LTC2991_V1_READY)
	{
		code_to_voltage(v1_msb, v1_lsb, &meas);
		op->voltage_1v0 = meas;
	}

	if (status_l & LTC2991_V2_READY)
	{
		code_to_voltage(v2_msb, v2_lsb, &meas);
		op->voltage_2v0 = meas;
	}

	if (status_l & LTC2991_V3_READY)
	{
		code_to_voltage(v3_msb, v3_lsb, &meas);
		op->voltage_3v0 = meas;
	}

	if (status_l & LTC2991_V4_READY)
	{
		code_to_voltage(v4_msb, v4_lsb, &meas);
		op->voltage_4v0 = meas;
	}

	SENSOR_U3_ERROR: return;
}

static void i2c_sensor_u4_read_nomutex(sensor_data_t *op)
{
	uint8_t status_l, status_h, msb, lsb, i5_msb, i5_lsb;
	uint8_t ret1, ret = 0;
	int32_t meas;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U4,
	LTC2991_STATUS_LOW_REG, &status_l);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U4,
	LTC2991_STATUS_HIGH_REG, &status_h);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U4,
	LTC2991_V3_LSB_REG, &lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U4,
	LTC2991_V3_MSB_REG, &msb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U4,
	LTC2991_V2_LSB_REG, &i5_lsb);
	ret = ret | ret1;

	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U4,
	LTC2991_V2_MSB_REG, &i5_msb);
	ret = ret | ret1;

	if (ret)
		goto SENSOR_U4_ERROR;

	if (status_l & LTC2991_V3_READY)
	{
		code_to_voltage(msb, lsb, &meas);
		op->voltage_5v0 = meas * 2; // double it
	}

	if (status_h & LTC2991_V12_READY)
	{
		code_to_current(i5_msb, i5_lsb, &meas);
		op->current_5v0 = meas;
	}
	trace_printf("5vis:%d\n", op->voltage_5v0);

	SENSOR_U4_ERROR: return;
}

void i2c_sensor_read(void)
{
	if (xSemaphoreTake(g_mutex_i2c_op, portMAX_DELAY) == pdTRUE)
	{
		i2c_sensor_u2_read_nomutex(&g_sensor_data);
		i2c_sensor_u3_read_nomutex(&g_sensor_data);
		i2c_sensor_u4_read_nomutex(&g_sensor_data);

		xSemaphoreGive(g_mutex_i2c_op);

	}

	return;
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
		code = (uint16_t) ((code ^ 0xfFFF) + 1); //! 1)Converts two's complement to binary
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

static HAL_StatusTypeDef i2c_sensor_I12_read(void)
{
	uint8_t data[2] =
	{ 0, 0 };
	uint8_t status_h;
	uint16_t code;
//	float temp;
	float voltage; //, current;
	int16_t sign;
	HAL_StatusTypeDef ret1, ret2;
	int32_t meas;

	//ret1 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_I12, LTC2991_STATUS_LOW_REG, &status_l);
	ret2 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_U2, LTC2991_STATUS_HIGH_REG,
			&status_h);

	if (HAL_OK != ret2)
		goto SENSOR_I12_ERROR;

	if (status_h & LTC2991_VCC_READY)
	{
		//read vcc
		ret1 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_U2, LTC2991_Vcc_LSB_REG,
				&data[0]);
		ret2 = i2c_sensor_read_reg(LTC2991_I2C_ADDRESS_U2, LTC2991_Vcc_MSB_REG,
				&data[1]);
		if ((HAL_OK != ret1) || (HAL_OK != ret2))
			goto SENSOR_I12_ERROR;

		code = (uint16_t) (data[0] | (data[1] << 8));

		if (code & (0x1 << 14))
		{
			trace_printf("vcc has a negative sign:::\n");
			code = (uint16_t) ((code ^ 0x7FFF) + 1); //! 1)Converts two's complement to binary
			sign = -1;
		}
		else
		{
			code = code & 0x3fff;
			sign = 1;
		}
		voltage = (float) ((code * LTC2991_VCC_lsb * sign + 2.5)
				* SENSOR_VOLT_SCALE); //! 2) Convert code to voltage form differential lsb
		if (-1 == sign)
			trace_printf("op->voltage_vcc = %f\n", (int) (voltage - 0.5));
		else
			trace_printf("op->voltage_vcc = %f\n", (int) (voltage + 0.5));
	}

	if (status_h & LTC2991_V12_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V2_MSB_REG, &meas);
		if (HAL_OK != ret1)
			goto SENSOR_I12_ERROR;
		trace_printf("op->current_1v0 = %d uA\n", meas);
	}

	if (status_h & LTC2991_V34_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V4_MSB_REG, &meas);
		if (HAL_OK != ret1)
			goto SENSOR_I12_ERROR;
		trace_printf("op->current_2v0 = %d uA\n", meas);
	}

	if (status_h & LTC2991_V56_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V6_MSB_REG, &meas);
		if (HAL_OK != ret1)
			goto SENSOR_I12_ERROR;
		trace_printf("op->current_3v0 = %d uA\n", meas);
	}

	if (status_h & LTC2991_V78_READY)
	{
		ret1 = i2c_sensor_meas_current(LTC2991_I2C_ADDRESS_U2,
		LTC2991_V8_MSB_REG, &meas);
		if (HAL_OK != ret1)
			goto SENSOR_I12_ERROR;
		trace_printf("op->current_4v0 = %d uA\n", meas);
	}

	return HAL_OK;
	SENSOR_I12_ERROR: return HAL_ERROR;
}

void vSensorHandlerTask(void *pvParameters)
{

	while (1)
	{
		//uint8_t ret1 = 0, ret2 = 0, ret3 = 0;
		i2c_sensor_config();

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		//		if (ret1 || ret2 || ret3)
		//		{
		//			i2c_sensor_config();
		//		}
		//		else
		//		{
		i2c_sensor_read();
		trace_printf("Voltages: S1:%05d, S2:%05d, S3:%05d, S4:%05d, S5:%05d\n",
				g_sensor_data.voltage_1v0, g_sensor_data.voltage_2v0,
				g_sensor_data.voltage_3v0, g_sensor_data.voltage_4v0,
				g_sensor_data.voltage_5v0);

		trace_printf("Currents: S1:%d, S2:%d, S3:%d, S4:%d, S5:%d\n",
				g_sensor_data.current_1v0, g_sensor_data.current_2v0,
				g_sensor_data.current_3v0, g_sensor_data.current_4v0,
				g_sensor_data.current_5v0);
		//		}
		i2c_sensor_I12_read();
	}
	return;
}

void vStartSensorTask(UBaseType_t uxPriority)
{
	BaseType_t xReturned;

	/* Spawn the task. */
	xReturned = xTaskCreate(vSensorHandlerTask, "SENSOR", SENSOR_STACK_SIZE,
	NULL, uxPriority, (TaskHandle_t*) &g_handle_sensor_task);
	if (xReturned != pdPASS)
	{
		trace_printf("failed to create sensor handling task\n");
		Error_Handler();
	}
	return;
}
