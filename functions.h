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
void ms_delay (uint32_t ms);													//wait for a given number of ms
void udef_GPIO_Pin_set_function (udef_GPIO_Pin *pins, uint8_t num_of_pins);		//inits pins, number of pins must be given
void GPIO_Pin_write (udef_GPIO_Pin *pin2set, uint32_t h_or_l);					//sets pin2set to a given state (HIGH or LOW)

#endif
