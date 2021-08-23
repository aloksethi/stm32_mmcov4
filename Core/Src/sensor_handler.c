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
//static const float LTC2991_DIFFERENTIAL_lsb = 1.90735E-05f;
//! Typical VCC LSB weight in volts
//static const float LTC2991_VCC_lsb = 3.05176E-04f;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
//static const float LTC2991_TEMPERATURE_lsb = 0.0625f;

static const uint32_t LTC2991_SE_LSB_MUL = 2500; // se voltage measuremetns are in mV
static const uint64_t LTC2991_DIFF_LSB_MUL = 25000000; // diff current measuremetns are in uA. sense resistor is 0.1 ohm
static const uint8_t LTC2991_SE_LSB_RHSIFT = 13;
static const uint8_t LTC2991_DIFF_LSB_RHSIFT = 17;

QueueHandle_t g_sensor_queue_handle;
static StaticQueue_t sensor_queue_ds;
static uint8_t sensor_queue_storage_area[sizeof(sensor_data_t*)]; // send a pointer on the queue

TaskHandle_t g_handle_sensor_task;
sensor_data_t g_sensor_data;

static uint8_t is_all_zero(void)
{
	int i;
	uint8_t *dptr = (uint8_t*) (&g_sensor_data);

	for (i = 0; i < sizeof(sensor_data_t); i++)
	{
		if (*dptr)
		{
			return 0;
		}
		dptr++;
	}
	return 1;
}
static uint8_t i2c_sensor_read_reg_nomutex(uint16_t addr, uint8_t reg,
		uint8_t *data)
{
	HAL_StatusTypeDef ret1, ret2;
	uint8_t ret = 0;

	ret1 = HAL_I2C_Master_Transmit(&g_hi2c1, addr, &reg, 1, 100);
	ret2 = HAL_I2C_Master_Receive(&g_hi2c1, addr, data, 1, 100);

	if ((HAL_OK != ret1) || (HAL_OK != ret2))
	{
		//trace_printf("failed to read sensor %x:%x \n", ret1, ret2);
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
		//trace_printf("failed to read sensor %x:%x \n", ret1, ret2);
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
		//	trace_printf("failed to set sensor %x \n", ret1);
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
//		trace_printf("I2C SENSOR U2 CONFIG V1234 FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_V5678_REG;
	data[1] = LTC2991_V5_V6_DIFFERENTIAL_ENABLE
			| LTC2991_V7_V8_DIFFERENTIAL_ENABLE | LTC2991_V5_V6_FILTER_ENABLE
			| LTC2991_V7_V8_FILTER_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U2, &data[0], 2);
	if (ret)
	{
		//	trace_printf("I2C SENSOR U2 CONFIG V5678 FAILED : %d\n", ret);
		return ret;
	}

	// enable all measurements
	data[0] = LTC2991_CHANNEL_ENABLE_REG;
	data[1] = LTC2991_ENABLE_ALL_CHANNELS;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U2, &data[0], 2);
	if (ret)
	{
//		trace_printf("I2C SENSOR CONFIG U2 CH_EN FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_PWM_Tinternal_REG;
	data[1] = LTC2991_REPEAT_MODE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U2, &data[0], 2);
	if (ret)
	{
//		trace_printf("I2C SENSOR CONFIG repeat mode FAILED : %d\n", ret);
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
//		trace_printf("I2C SENSOR U3 CONFIG V1234 FAILED : %d\n", ret);
		return ret;
	}

	// enable all measurements
	data[0] = LTC2991_CHANNEL_ENABLE_REG;
	data[1] = LTC2991_V1_V2_TR1_ENABLE | LTC2991_V3_V4_TR2_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U3, &data[0], 2);
	if (ret)
	{
//		trace_printf("I2C SENSOR CONFIG U3 CH_EN FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_PWM_Tinternal_REG;
	data[1] = LTC2991_REPEAT_MODE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U3, &data[0], 2);
	if (ret)
	{
//		trace_printf("I2C SENSOR CONFIG repeat mode FAILED : %d\n", ret);
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
//		trace_printf("I2C SENSOR U4 CONFIG V1234 FAILED : %d\n", ret);
		return ret;
	}

	// enable all measurements
	data[0] = LTC2991_CHANNEL_ENABLE_REG;
	data[1] = LTC2991_V1_V2_TR1_ENABLE | LTC2991_V3_V4_TR2_ENABLE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U4, &data[0], 2);
	if (ret)
	{
//		trace_printf("I2C SENSOR CONFIG U4 CH_EN FAILED : %d\n", ret);
		return ret;
	}

	data[0] = LTC2991_CONTROL_PWM_Tinternal_REG;
	data[1] = LTC2991_REPEAT_MODE;
	ret = i2c_sensor_set_reg(LTC2991_I2C_ADDRESS_U4, &data[0], 2);
	if (ret)
	{
//		trace_printf("I2C SENSOR CONFIG repeat mode FAILED : %d\n", ret);
		return ret;
	}
	return ret;
}

static uint8_t i2c_sensor_config(void)
{
	uint8_t ret = 0, ret1;
	ret1 = i2c_sensor_config_u2();
	ret |= ret1;

	ret1 = i2c_sensor_config_u3();
	ret |= ret1;

	ret1 = i2c_sensor_config_u4();
	ret |= ret1;

	return ret;
}

static void code_to_voltage(uint8_t msb, uint8_t lsb, uint32_t *val)
{
	uint32_t voltage;
	int8_t sign;
	uint16_t code;

	code = (uint16_t) (lsb | ((uint16_t) msb << 8));
	code = code & 0x7fff; // data is 15 bit and 15th bit is the sign bit

	if (code & (0x1 << 14))
	{
		code = (uint16_t) ((code ^ 0x7FFF) + 1); //! 1)Converts two's complement to binary
		sign = -1;
	}
	else
	{
		//code = code & 0x3fff;
		sign = 1;
	}
	//trace_printf("V: sign = %d, code = %d, lsb = %d, msb = %d\n", sign, code,
	//		lsb, msb);
	voltage = ((uint32_t) code * LTC2991_SE_LSB_MUL) >> LTC2991_SE_LSB_RHSIFT;

	*val = (uint32_t) (voltage & 0xffffffff);	// * sign;

	return;
}

static void code_to_current(uint8_t msb, uint8_t lsb, uint32_t *val)
{
	uint64_t current;
	int8_t sign;
	uint16_t code;

	code = (uint16_t) (lsb) | ((uint16_t) msb << 8);
	code = code & 0x7fff; // data is 15 bit and 15th bit is the sign bit
	if (code & (0x1 << 14))
	{
		code = (uint16_t) ((code ^ 0x7FFF) + 1); //! 1)Converts two's complement to binary
		sign = -1;
	}
	else
	{
		//code = code & 0x3fff;
		sign = 1;
	}
	trace_printf("I sign = %d, code = %d, lsb = %d, msb = %d\n", sign, code,
			lsb, msb);

	current = (uint64_t) code * LTC2991_DIFF_LSB_MUL;
	current = current >> LTC2991_DIFF_LSB_RHSIFT;

	*val = (uint32_t) (current & 0xffffffff); // * sign;

	return;
}

static uint8_t i2c_sensor_u2_read_nomutex(sensor_data_t *op)
{
	uint8_t status_h;
	uint8_t ret1, ret = 0;
	uint32_t meas;
	uint8_t v2_msb, v2_lsb, v4_msb, v4_lsb, v6_msb, v6_lsb, v8_msb, v8_lsb;
	board_red_led_toggle();
//	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
//	LTC2991_STATUS_HIGH_REG, &status_h);
	ret1 = i2c_sensor_read_reg_nomutex(LTC2991_I2C_ADDRESS_U2,
	LTC2991_STATUS_LOW_REG, &status_h);
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
	else
		op->current_1v0 = 0;

	if (status_h & LTC2991_V34_READY)
	{
		code_to_current(v4_msb, v4_lsb, &meas);

		op->current_2v0 = meas;
	}
	else
		op->current_2v0 = 0;

	if (status_h & LTC2991_V56_READY)
	{
		code_to_current(v6_msb, v6_lsb, &meas);

		op->current_3v0 = meas;
	}
	else
		op->current_3v0 = 0;

	if (status_h & LTC2991_V78_READY)
	{
		code_to_current(v8_msb, v8_lsb, &meas);

		op->current_4v0 = meas;
	}
	else
		op->current_4v0 = 0;

	board_red_led_toggle();
	SENSOR_U2_ERROR: return ret;
}

static uint8_t i2c_sensor_u3_read_nomutex(sensor_data_t *op)
{
	uint8_t status_l;
	uint8_t ret1, ret = 0;
	uint32_t meas;
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
	else
		op->voltage_1v0 = 0;

	if (status_l & LTC2991_V2_READY)
	{
		code_to_voltage(v2_msb, v2_lsb, &meas);
		op->voltage_2v0 = meas;
	}
	else
		op->voltage_2v0 = 0;

	if (status_l & LTC2991_V3_READY)
	{
		code_to_voltage(v3_msb, v3_lsb, &meas);
		op->voltage_3v0 = meas;
	}
	else
		op->voltage_3v0 = 0;

	if (status_l & LTC2991_V4_READY)
	{
		code_to_voltage(v4_msb, v4_lsb, &meas);
		op->voltage_4v0 = meas;
	}
	else
		op->voltage_4v0 = 0;

	SENSOR_U3_ERROR: return ret;
}

static uint8_t i2c_sensor_u4_read_nomutex(sensor_data_t *op)
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
	else
		op->voltage_5v0 = 0;

//	if (status_h & LTC2991_V12_READY)
	if (status_l & LTC2991_V12_READY)
	{
		code_to_current(i5_msb, i5_lsb, &meas);
		op->current_5v0 = meas;
	}
	else
		op->current_5v0 = 0;

	//trace_printf("5vis:%d\n", op->voltage_5v0);

	SENSOR_U4_ERROR: return ret;
}

static uint8_t i2c_sensor_read(void)
{
	uint8_t ret1, ret = 0;

	if (xSemaphoreTake(g_mutex_i2c_op, portMAX_DELAY) == pdTRUE)
	{
		ret1 = i2c_sensor_u2_read_nomutex(&g_sensor_data);
		ret = ret | ret1;

		ret1 = i2c_sensor_u3_read_nomutex(&g_sensor_data);
		ret = ret | ret1;

		ret1 = i2c_sensor_u4_read_nomutex(&g_sensor_data);
		ret = ret | ret1;

		xSemaphoreGive(g_mutex_i2c_op);

	}

	return ret;
}

void vSensorHandlerTask(void *pvParameters)
{
	static uint8_t sensor_not_configed = 1;
	uint8_t ret = 0;
	while (1)
	{
//		1 = 0, ret2 = 0, ret3 = 0;
		if (sensor_not_configed)
		{
			sensor_not_configed = i2c_sensor_config();
			if (sensor_not_configed)
			{
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				continue;
			}
		}

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		//		if (ret1 || ret2 || ret3)
		//		{
		//			i2c_sensor_config();
		//		}
		//		else
		//		{
		ret = i2c_sensor_read();
		if (ret)
		{
			sensor_not_configed = 1;
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}
		unsigned long ptr_to_sensor_data = &g_sensor_data;
		xQueueOverwrite(g_sensor_queue_handle, (void* )&ptr_to_sensor_data);
		if (is_all_zero())
		{
			sensor_not_configed = 1;
		}
//		trace_printf("Voltages: S1:%05d, S2:%05d, S3:%05d, S4:%05d, S5:%05d\n",
//				g_sensor_data.voltage_1v0, g_sensor_data.voltage_2v0,
//				g_sensor_data.voltage_3v0, g_sensor_data.voltage_4v0,
//				g_sensor_data.voltage_5v0);
//
//		trace_printf("Currents: S1:%d, S2:%d, S3:%d, S4:%d, S5:%d\n",
//				g_sensor_data.current_1v0, g_sensor_data.current_2v0,
//				g_sensor_data.current_3v0, g_sensor_data.current_4v0,
//				g_sensor_data.current_5v0);
		//		}
		//i2c_sensor_I12_read();
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
	g_sensor_queue_handle = xQueueCreateStatic(1, sizeof(sensor_data_t*),
			sensor_queue_storage_area, &sensor_queue_ds);
	if ( NULL == g_sensor_queue_handle)
	{
		trace_printf("failed to create the sensor queue\n");
		Error_Handler();
	}
	return;
}
