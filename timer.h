/**
 * @file timer.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2023-09-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __TIMER_H__
#define __TIMER_H__

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 
 * Configure a timer peripheral to generate interrupts.  This function will configure
 * the timer as a Count-Up timer that runs at 100MHz 
 * @param timer_obj 
 * Timer handle that will be used to access the timer.  
 * @param timer_cfg 
 * timer_cfg handle that will be used to configure the timer
 * @param ticks 
 * Defines the rate at which interrupts are geneated.  
 * @param Handler 
 * Function pointer the the Timer ISR (Callback)
 */
void timer_init(cyhal_timer_t *timer_obj, cyhal_timer_cfg_t *timer_cfg, uint32_t ticks, void *Handler);

#endif