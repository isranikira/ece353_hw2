/**
 * @file io-leds.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-09-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "io-leds.h"


/*****************************************************************************/
/* ICE 03 START */
/*****************************************************************************/
void leds_init(void)
{
    cy_rslt_t rslt;

    rslt = cyhal_gpio_init(PIN_RGB_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    CY_ASSERT(rslt == CY_RSLT_SUCCESS);

    rslt = cyhal_gpio_init(PIN_RGB_GRN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    CY_ASSERT(rslt == CY_RSLT_SUCCESS);

    rslt = cyhal_gpio_init(PIN_RGB_BLU, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    CY_ASSERT(rslt == CY_RSLT_SUCCESS);

}
/*****************************************************************************/
/* ICE 03 END */
/*****************************************************************************/



/*****************************************************************************/
/* ICE 06 START                                                              */
/*****************************************************************************/

/*
* Global variables that are used to interact with the PWM peripherals for
* the 3 pins that control the RGB LED.
*/

cyhal_pwm_t pwm_red_obj;
cyhal_pwm_t pwm_green_obj;
cyhal_pwm_t pwm_blue_obj;

/**
 * @brief 
 *  Initializes the RGB LED pins to be controlled by thier PWM peripherals 
 */
void leds_pwm_init(void)
{
    cy_rslt_t rslt;

    rslt = cyhal_pwm_init(&pwm_red_obj, PIN_RGB_RED, NULL);
    CY_ASSERT(CY_RSLT_SUCCESS == rslt);

    rslt = cyhal_pwm_init(&pwm_green_obj, PIN_RGB_GRN, NULL);
    CY_ASSERT(CY_RSLT_SUCCESS == rslt);

    rslt = cyhal_pwm_init(&pwm_blue_obj, PIN_RGB_BLU, NULL);
    CY_ASSERT(CY_RSLT_SUCCESS == rslt);
}
/*****************************************************************************/
/* ICE 06 END                                                                */
/*****************************************************************************/