/*
 * mlab_handler.c
 *
 *  Created on: Jan 26, 2021
 *      Author: asethi
 */

#include <string.h>
#include "main.h"
#include "mlab_handler.h"

#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"

#include "queue.h"

static uint8_t g_rcv_buffer[2048]; // biggest payload

static TaskHandle_t g_handle_mlab_task;
static sensor_data_t *sensor_queue_ptr;

static uint16_t handle_command(mlab_data_t *raw_data_p)
{
	uint16_t data_to_return = 0;

	switch (raw_data_p->command_code)
	{

	case G_UC_SET_REG_CONFIG:
	{
		uint8_t ic_id;
		uint8_t i, num_chunks; //number of chunks of sub data
		reg_t *local_reg_ptr;

		pc_queue_data_t queue_post;

		trace_printf("command: set_reg \n");

		ic_id = raw_data_p->ic_id;
		num_chunks = raw_data_p->num_chunks;
		for (i = 0; i < num_chunks; i++)
		{
			local_reg_ptr = (reg_t*) (&raw_data_p->data[0] + i * sizeof(reg_t));
			pc_set_curr_value(ic_id, local_reg_ptr);

			queue_post.command_code = G_UC_SET_REG_CONFIG;
			queue_post.ic_id = ic_id;
			queue_post.reg_id = local_reg_ptr->reg_id;

			xQueueSendToBack(g_pc_queue_handle, &queue_post, portMAX_DELAY);
		}
		break;
	}

	case G_UC_SAVE_DEF_REG_CONFIG:
	{
		uint8_t ic_id;
		uint8_t i, num_chunks; //number of chunks of sub data
		reg_t *local_reg_ptr;

		trace_printf("command: save_def \n");

		ic_id = raw_data_p->ic_id;
		num_chunks = raw_data_p->num_chunks;
		for (i = 0; i < num_chunks; i++)
		{
			local_reg_ptr = (reg_t*) (&raw_data_p->data[0] + i * sizeof(reg_t));
			pc_save_default_value(ic_id, local_reg_ptr);
		}
		break;
	}

	case G_UC_SAVE_MINI_REG_CONFIG:
	{
		uint8_t ic_id;
		uint8_t i, num_chunks; //number of chunks of sub data
		reg_t *local_reg_ptr;

		trace_printf("command: save_mini \n");

		ic_id = raw_data_p->ic_id;
		num_chunks = raw_data_p->num_chunks;
		for (i = 0; i < num_chunks; i++)
		{
			local_reg_ptr = (reg_t*) (&raw_data_p->data[0] + i * sizeof(reg_t));
			pc_save_mini_value(ic_id, local_reg_ptr);
		}
		break;
	}

	case G_UC_SAVE_MAXI_REG_CONFIG:
	{
		uint8_t ic_id;
		uint8_t i, num_chunks; //number of chunks of sub data
		reg_t *local_reg_ptr;

		trace_printf("command: save_maxi \n");

		ic_id = raw_data_p->ic_id;
		num_chunks = raw_data_p->num_chunks;
		for (i = 0; i < num_chunks; i++)
		{
			local_reg_ptr = (reg_t*) (&raw_data_p->data[0] + i * sizeof(reg_t));
			pc_save_maxi_value(ic_id, local_reg_ptr);
		}
		break;
	}

	case G_UC_APPLY_DEF_REG_CONFIG:
	{
		uint8_t ic_id;
		pc_queue_data_t queue_post;

		trace_printf("command: apply_def \n");

		ic_id = raw_data_p->ic_id;
		queue_post.command_code = G_UC_APPLY_DEF_REG_CONFIG;
		queue_post.ic_id = ic_id;
//			queue_post.reg_id = local_reg_ptr->reg_id;

		xQueueSendToBack(g_pc_queue_handle, &queue_post, portMAX_DELAY);
		break;
	}

	case G_UC_APPLY_MAXI_REG_CONFIG:
	{
		uint8_t ic_id;
		pc_queue_data_t queue_post;

		trace_printf("command: apply_maxi \n");

		ic_id = raw_data_p->ic_id;
		queue_post.command_code = G_UC_APPLY_MAXI_REG_CONFIG;
		queue_post.ic_id = ic_id;
//			queue_post.reg_id = local_reg_ptr->reg_id;

		xQueueSendToBack(g_pc_queue_handle, &queue_post, portMAX_DELAY);
		break;
	}

	case G_UC_APPLY_MINI_REG_CONFIG:
	{
		uint8_t ic_id;
		pc_queue_data_t queue_post;

		trace_printf("command: apply_def \n");

		ic_id = raw_data_p->ic_id;
		queue_post.command_code = G_UC_APPLY_MINI_REG_CONFIG;
		queue_post.ic_id = ic_id;
//			queue_post.reg_id = local_reg_ptr->reg_id;

		xQueueSendToBack(g_pc_queue_handle, &queue_post, portMAX_DELAY);
		break;
	}

	case G_UC_SYNTH_POWER:
	{
		uint32_t val;

		trace_printf("command: synth_power \n");
		val = (uint32_t) (raw_data_p->data); // make power value out of pointer
		if (val)
			board_synth_power_on();
		else
			board_synth_power_off();

		break;
	}

	case G_UC_SYNTH_CONFIG:
	{
		trace_printf("command: synth_config \n");

		break;
	}

	case G_UC_3V3_POWER:
	{
		uint32_t val;

		trace_printf("command: 3V3_power \n");
		val = (uint32_t) (raw_data_p->data); // make power value out of pointer
		if (val)
			board_3v3_power_on();
		else
			board_3v3_power_off();

		break;
	}

	case G_UC_PB_SUP_EN:
	case G_UC_PB_SUP_VAL:
	{
		pot_data_t *local_reg_ptr;
		uint8_t i, num_chunks; //number of chunks of sub data

		trace_printf("command: power_board_supply \n");

		pot_queue_data_t queue_post;
		num_chunks = raw_data_p->num_chunks;
		for (i = 0; i < num_chunks; i++)
		{
			local_reg_ptr = (pot_data_t*) (&raw_data_p->data[0]
					+ i * sizeof(pot_data_t));

			queue_post.command_code = raw_data_p->command_code;
			queue_post.pot_data.sup_id = local_reg_ptr->sup_id;
			queue_post.pot_data.sup_status = local_reg_ptr->sup_status;
			queue_post.pot_data.pot_val = local_reg_ptr->pot_val;

			xQueueSendToBack(g_pot_queue_handle, &queue_post, portMAX_DELAY);
		}
		break;
	}

	case G_UC_PB_SENSORS:
	{
		trace_printf("command: get pb sensors\n");
		xTaskNotifyGive(g_handle_sensor_task);

		if (xQueueReceive(g_sensor_queue_handle, &sensor_queue_ptr,
				(300 / portTICK_PERIOD_MS)) == pdPASS)
		{
			data_to_return = sizeof(sensor_data_t);
		}
		else
			data_to_return = 0;

		break;
	}

	}

	return data_to_return;
}
static uint8_t sanity_check(mlab_data_t *raw_data_p)
{
	uint8_t sane = 0;

	if ((raw_data_p->command_code <= G_UC_MIN_COMMAND_CODE)
			|| (raw_data_p->command_code >= G_UC_MAX_COMMAND_CODE))
	{
		trace_printf("invalid command code : %d\n", raw_data_p->command_code);
		sane = 0;
		return sane;
	}

	if ((raw_data_p->ic_id == 0) || (raw_data_p->ic_id > G_MAX_ICS_PER_UC))
	{
		trace_printf("invalid ic_id\n");
		sane = 0;
		return sane;
	}

	if ((raw_data_p->num_chunks > G_MAX_NUM_REGS)) //num_chunks is 0 for apply_preset_config
	{
		trace_printf("invalid num_chunks\n");
		sane = 0;
		return sane;
	}

	if (raw_data_p->command_code == G_UC_SET_REG_CONFIG)
	{
		uint8_t num_regs, i;

		num_regs = raw_data_p->num_chunks;

		if (num_regs == 0)
		{
			trace_printf("invalid number of registers in the buffer\n");
			sane = 0;
			return sane;
		}
		for (i = 0; i < num_regs; i++)
		{
			reg_t *local_reg_ptr;
			uint8_t reg_id, cascade;

			local_reg_ptr = (reg_t*) (&raw_data_p->data[0] + i * sizeof(reg_t));
			reg_id = local_reg_ptr->reg_id;
			cascade = local_reg_ptr->cascade;

			if ((cascade > G_REGS_PER_REG) || (reg_id > G_MAX_NUM_REGS))
			{
				trace_printf("invalid arguments\n");
				sane = 0;
				return sane;
			}
		}
	}

	if (raw_data_p->command_code == G_UC_PB_SUP_EN)
	{
		pot_data_t *local_reg_ptr;
		uint8_t i, num_chunks, sup_id; //number of chunks of sub data

		num_chunks = raw_data_p->num_chunks;
		for (i = 0; i < num_chunks; i++)
		{
			local_reg_ptr = (pot_data_t*) (&raw_data_p->data[0]
					+ i * sizeof(pot_data_t));

			sup_id = local_reg_ptr->sup_id;
			if ((sup_id == 0) || (sup_id > G_PB_SUP_ALL))
			{
				sane = 0;
				trace_printf("invalid supply id\n");
				return sane;
			}

		}
	}
	if (raw_data_p->command_code == G_UC_PB_SUP_VAL)
	{
		pot_data_t *local_reg_ptr;
		uint8_t i, num_chunks, sup_id; //number of chunks of sub data

		num_chunks = raw_data_p->num_chunks;
		for (i = 0; i < num_chunks; i++)
		{
			local_reg_ptr = (pot_data_t*) (&raw_data_p->data[0]
					+ i * sizeof(pot_data_t));

			sup_id = local_reg_ptr->sup_id;
			if ((sup_id == 0) || (sup_id >= G_PB_SUP_ALL))
			{
				sane = 0;
				trace_printf("invalid supply id\n");
				return sane;
			}

		}
	}
	sane = 1;
	return sane;
}
static portTASK_FUNCTION( vMlabHandlerTask, pvParameters )
{
	/* The parameters are not used. */
	(void) pvParameters;

	static struct netconn *conn;
	static struct netbuf *buf;
	static ip_addr_t *addr;
	static unsigned short port;
	err_t err;
	int8_t ret_data[4];
	void *ptr_payload = NULL;

	conn = netconn_new(NETCONN_UDP);
	LWIP_ASSERT("con != NULL", conn != NULL);
	netconn_bind(conn, NULL, G_NUCLEO_PORT_NUM);

	while (1)
	{
		err = netconn_recv(conn, &buf);
		if (err == ERR_OK)
		{
			mlab_data_t *raw_data_p;
			uint8_t sane;
			uint16_t len;
			uint32_t running_id;
			uint16_t data_to_return;
			uint16_t tot_amt_to_return;

			addr = netbuf_fromaddr(buf);
			port = netbuf_fromport(buf);
			//netconn_connect(conn, addr, port);
			if (sizeof(g_rcv_buffer) <= buf->p->tot_len)
			{
				trace_printf("total packet bigger than available buffer\n");
				Error_Handler();
			}
			netbuf_copy(buf, g_rcv_buffer, buf->p->tot_len);

			trace_printf("rcvd udp, src_port:%d, ipaddr:", port);
			ip4_addr_debug_print(LWIP_DBG_ON, addr);
			trace_printf("\n");

			//g_rcv_buffer[buf->p->tot_len] = '\0';

			raw_data_p = (mlab_data_t*) (&g_rcv_buffer[0]);
			running_id = ntohl(raw_data_p->running_id);

			sane = sanity_check(raw_data_p);

			if (sane)
			{
				strcpy(ret_data, "OK");
				data_to_return = handle_command(raw_data_p);
			}
			else
			{
				strcpy(ret_data, "NOK");
			}

			netbuf_data(buf, &ptr_payload, &len);
			//trace_printf("len of buf=%d\n", len);
			tot_amt_to_return = data_to_return + sizeof(running_id)
					+ sizeof(ret_data);
			if (len < tot_amt_to_return)
			{
				//trace_printf("FIXME:less bytes availabel");
				netbuf_free(buf);
				ptr_payload = netbuf_alloc(buf, tot_amt_to_return);
			}

			if (NULL != ptr_payload)
			{
				*(uint32_t*) (ptr_payload) = htonl(running_id);
//				memcpy(ptr_payload + sizeof(running_id), ret_data,
//						sizeof(ret_data) + sizeof(uint32_t));
				memcpy(ptr_payload + sizeof(running_id), ret_data,
						sizeof(ret_data));
				if (data_to_return)
				{
					memcpy(ptr_payload + sizeof(running_id) + sizeof(ret_data),
							sensor_queue_ptr, sizeof(sensor_data_t));
				}
				buf->p->tot_len = tot_amt_to_return;
				buf->p->len = tot_amt_to_return;

				netconn_send(conn, buf);
				//LWIP_DEBUGF(LWIP_DBG_ON, ("got %s\n", buffer));
				netbuf_delete(buf);

			}
			else
			{
				trace_printf("unable to allocate return data \n");
			}

		}
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

void vStartMlabHandlerTask(UBaseType_t uxPriority)
{
	BaseType_t xReturned;

	xReturned = xTaskCreate(vMlabHandlerTask, "MLABHx",
	MATLAB_HANLDER_STACK_SIZE, NULL, uxPriority,
			(TaskHandle_t*) &g_handle_mlab_task);
	if (xReturned != pdPASS)
	{
		/* The task was created.  Use the task's handle to delete the task. */
		trace_printf("failed to create the matlab handler task\n");
	}
}
