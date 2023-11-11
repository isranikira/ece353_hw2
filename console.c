/**
 * @file ece353.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-08-15
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "console.h"

extern cyhal_uart_t cy_retarget_io_uart_obj;

/*****************************************************************************/
/* ICE 01 START*/
/*****************************************************************************/
void console_init(void)
{
    cy_rslt_t rslt;

    /* Initialize retarget-io to use the debug UART port, 8N1 */
    rslt = cy_retarget_io_init(
        PIN_UART_DBG_TX,
        PIN_UART_DBG_RX,
        115200); // Baud Rate

    // If the initialization of the console fails, halt the MCU
    CY_ASSERT(rslt == CY_RSLT_SUCCESS);
}
/*****************************************************************************/
/* ICE 01 END */
/*****************************************************************************/

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
bool console_rx_string(uint8_t *msg)
{
    static uint8_t temp_buffer[80];
    static uint8_t buffer_index = 0;

    cy_rslt_t rslt;
    uint8_t c;
    bool return_value = false;

    /* Check to see if there is a new character from the console*/
    /* Wait for 1ms if no character has been received */
    rslt = cyhal_uart_getc(&cy_retarget_io_uart_obj, &c, 1);

    if (rslt == CY_RSLT_SUCCESS)
    {
        /* Add the current character to the message*/
        temp_buffer[buffer_index] = c;

        /* If the character returned is a \n, return true*/
        if (c == '\n' || c == '\r')
        {
            temp_buffer[buffer_index] = 0;
            /* Copy the message to the destination address*/
            strcpy(msg, temp_buffer);

            buffer_index = 0;

            return_value = true;

            /* Erase all characters and reset the index */
        }

        else
        {
            buffer_index++;
        }
    }
    return return_value;
}