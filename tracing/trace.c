/*
 * trace.c
 *
 *  Created on: Jan 21, 2021
 *      Author: asethi
 */

#include <stdio.h>
#include <stdarg.h>
#include "stm32f4xx.h"

#define OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE (128)

void trace_init(void)
{
//	CoreDebug->DEMCR |= _VAL2FLD(CoreDebug_DEMCR_TRCENA, 1);
//	TPI->SPPR |= _VAL2FLD(TPI_SPPR_TXMODE, 2);
//	DBGMCU->CR  |= _VAL2FLD(DBGMCU_CR_TRACE_IOEN, DBGMCU_CR_TRACE_IOEN);
//	ITM->TER = 1;
	return;
}
static ssize_t trace_write (const char* buf, size_t nbyte)
{
  for (size_t i = 0; i < nbyte; i++)
    {

      ITM_SendChar(*(buf+i));
    }

  return (ssize_t)nbyte; // all characters successfully sent
}

int trace_printf(const char* format, ...)
{
  int ret;
  va_list ap;

  va_start (ap, format);

  static char buf[OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE];

  // Print to the local buffer
  ret = vsnprintf (buf, sizeof(buf), format, ap);
  if (ret > 0)
    {
      // Transfer the buffer to the device
      ret = trace_write (buf, (size_t)ret);
    }

  va_end (ap);
  return ret;
}

