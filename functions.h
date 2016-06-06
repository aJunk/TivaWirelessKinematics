#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

typedef struct udef_GPIO_Pin{
	uint32_t peripheral;
	uint32_t pin_type;
	uint32_t port_base;
	uint8_t pin;
}udef_GPIO_Pin;

/* ----------------------- DEFINES ----------------------- */
/* GENERAL */
#define LOW 0
#define HIGH UINT8_MAX
#define OUTPUT GPIO_PIN_TYPE_STD
#define INPUT GPIO_PIN_TYPE_STD_WPU

/* TIMER */
#define F_CPU 120000000

/* ----------------------- GLOBAL VARIABLES ----------------------- */
/* LEDS */
extern udef_GPIO_Pin leds[4];

/* BUTTONS */
extern udef_GPIO_Pin buttons[2];

/* ----------------------- FUNCTION PROTOTYPES ----------------------- */
void ms_delay (uint32_t ms);
/*Waits for a given number milliseconds
* Parameters:	uint32_t ms		is the number of milliseconds to wait
* Return value:	none
*/

void udef_GPIO_Pin_set_function (udef_GPIO_Pin *pins, uint8_t num_of_pins);
/*Calls all necessary functions to initialize given pins. Pin-type, port-base and pin must be saved in struct udef_GPIO_Pin
* Parameters: 	udef_GPIO_Pin *pins		is the struct containing all pins to be initialized
				uint8_t num_of_pins		is the number of pins in *pins to be initialized
* Return value:	none
*/

void GPIO_Pin_write (udef_GPIO_Pin *pin2set, uint8_t h_or_l);
/*
* Parameters:	udef_GPIO_Pin *pins		is the struct containing all pin-information of the pin which should be set
				uint8_t h_or_l			is the state the pin should be set to. Use defines HIGH or LOW
* Return value:	none
*/

#endif
