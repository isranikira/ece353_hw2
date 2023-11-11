/**
 * @file ece353.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-08-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include <ctype.h>
#include <stdio.h>

/* UART Pins*/
#define PIN_UART_DBG_RX			P5_0
#define PIN_UART_DBG_TX			P5_1

/**
 * @brief 
 * Initialize the UART interface used to print messages to the serial monitor  
 */
void console_init(void);

/**
 * @brief 
 * Returns a string entered from the console 
 * @param msg 
 * A pointer to the character array where the string will be written to
 * @return true
 * A string was entered at the console.  The string ends when a \n or \r is
 * received
 * @return false 
 * A string has not been received.
 */
bool console_rx_string(uint8_t *msg);

#endif