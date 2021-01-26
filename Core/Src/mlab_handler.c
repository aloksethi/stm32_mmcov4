/*
 * mlab_handler.c
 *
 *  Created on: Jan 26, 2021
 *      Author: asethi
 */

#include "main.h"
#include "mlab_handler.h"

#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"

	 	  char buffer[4096];

static portTASK_FUNCTION( vMlabHandlerTask, pvParameters )
{
	/* The parameters are not used. */
	( void ) pvParameters;

	 static struct netconn *conn;
	 	  static struct netbuf *buf;
	 	  static ip_addr_t *addr;
	 	  static unsigned short port;
	 	  err_t err;

	 	  conn = netconn_new(NETCONN_UDP);
	 	  LWIP_ASSERT("con != NULL", conn != NULL);
	 	  netconn_bind(conn, NULL, 4040);

	 	  while (1)
	 	  {
	 	    err = netconn_recv(conn, &buf);
	 	    if (err == ERR_OK) {
	 	      addr = netbuf_fromaddr(buf);
	 	      port = netbuf_fromport(buf);
	 	      netconn_connect(conn, addr, port);
	 	      netbuf_copy(buf, buffer, buf->p->tot_len);
	 	      buffer[buf->p->tot_len] = '\0';
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

	xReturned = xTaskCreate( vMlabHandlerTask, "MLABHx", MATLAB_HANLDER_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
	if( xReturned != pdPASS )
	{
		/* The task was created.  Use the task's handle to delete the task. */
		trace_printf("failed to create the matlab handler task\n");
	}
}
