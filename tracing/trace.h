/*
 * trace.h
 *
 *  Created on: Jan 21, 2021
 *      Author: asethi
 */

#ifndef TRACE_H_
#define TRACE_H_

#define TRACE_BUFF (64)

void trace_init(void);
int trace_printf(const char* format, ...);


#endif /* TRACE_H_ */
