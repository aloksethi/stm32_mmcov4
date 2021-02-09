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

uint8_t g_rcv_buffer[2048]; // biggest payload

void handle_command( mlab_data_t *raw_data_p )
{
	switch( raw_data_p->command_code )
	{

	case G_UC_SET_REG_CONFIG:
	{
		uint8_t ic_id;
		uint8_t i, num_chunks; //number of chunks of sub data
		reg_t * local_reg_ptr;

		pc_queue_data_t pc_queue_local_copy;

		ic_id = raw_data_p->ic_id;
		num_chunks = raw_data_p->num_chunks;
		for (i=0; i<num_chunks; i++)
		{
			local_reg_ptr = (reg_t *)(&raw_data_p->data[0] + i*sizeof(reg_t));
			pc_set_curr_value(ic_id, local_reg_ptr);

			pc_queue_local_copy.command_code = G_UC_SET_REG_CONFIG;
			pc_queue_local_copy.ic_id = ic_id;
			pc_queue_local_copy.reg_id = local_reg_ptr->reg_id;
		 xQueueSendToBack(g_pc_queue_handle, &pc_queue_local_copy, portMAX_DELAY);
		}
		break;
	}

	}

	return;
}
uint8_t sanity_check( mlab_data_t *raw_data_p )
{
	uint8_t sane=0;

	if ((raw_data_p->command_code < G_UC_APPLY_DEF_REG_CONFIG) ||
			(raw_data_p->command_code >= G_UC_MAX_COMMAND_CODE))
	{
		trace_printf("invalid command code : %d\n",raw_data_p->command_code);
		sane = 0;
		return sane;
	}

	if (raw_data_p->command_code == G_UC_SET_REG_CONFIG)
	{
		uint8_t num_regs, i;

		trace_printf("command: set_reg \n");

		num_regs = raw_data_p->num_chunks;

		if (num_regs > G_MAX_NUM_REGS)
		{
			trace_printf("invalid number of registers in the buffer\n");
			sane = 0;
			return sane;
		}
		for (i=0; i<num_regs; i++)
		{
			reg_t * local_reg_ptr;
			uint8_t reg_id, cascade;

			local_reg_ptr = (reg_t *)(&raw_data_p->data[0] + i*sizeof(reg_t));
			reg_id = local_reg_ptr->reg_id;
			cascade = local_reg_ptr->cascade;

			if ((cascade > G_REGS_PER_REG) || (reg_id > G_MAX_NUM_REGS))
			{
				trace_printf("invalid arguments\n");
				sane = 1; //TODO: change sane to 0 here
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
	( void ) pvParameters;

	static struct netconn *conn;
	static struct netbuf *buf;
	static ip_addr_t *addr;
	static unsigned short port;
	err_t err;
	uint8_t ret_data[16];
	void * ptr_payload = NULL;

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

			addr = netbuf_fromaddr(buf);
			port = netbuf_fromport(buf);
			netconn_connect(conn, addr, port);
			if (sizeof(g_rcv_buffer) <= buf->p->tot_len)
			{
				trace_printf("total packet bigger than available buffer\n");
				Error_Handler();
			}
			netbuf_copy(buf, g_rcv_buffer, buf->p->tot_len);

			//g_rcv_buffer[buf->p->tot_len] = '\0';

			raw_data_p = (mlab_data_t *)(&g_rcv_buffer[0]);

			sane = sanity_check(raw_data_p);

			if (sane)
			{
				strcpy(ret_data, "OK");
				handle_command(raw_data_p);
			}
			else
			{
				strcpy(ret_data, "NOT OK");

			}

			netbuf_data(buf, &ptr_payload, &len);

			if (NULL != ptr_payload)
			{
				memcpy(ptr_payload, ret_data, sizeof(ret_data));
			}
			else
			{
				trace_printf("unable to allocate return data \n");
			}
			buf->p->tot_len = 16;
			buf->p->len = 16;

			netconn_send(conn, buf);
			trace_printf("rcvd udp, src_port:%d, ipaddr:",port);
			ip4_addr_debug_print(LWIP_DBG_ON, addr);
			trace_printf("\n");
			//LWIP_DEBUGF(LWIP_DBG_ON, ("got %s\n", buffer));
			netbuf_delete(buf);

		}
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */


void vStartMlabHandlerTask( UBaseType_t uxPriority )
{
	BaseType_t xReturned;

	xReturned = xTaskCreate( vMlabHandlerTask, "MLABHx", MATLAB_HANLDER_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) &g_handle_mlab_task );
	if( xReturned != pdPASS )
	{
		/* The task was created.  Use the task's handle to delete the task. */
		trace_printf("failed to create the matlab handler task\n");
	}
}
