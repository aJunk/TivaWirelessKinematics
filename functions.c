#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"

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
udef_GPIO_Pin leds[4] = {	{SYSCTL_PERIPH_GPION, OUTPUT, GPIO_PORTN_BASE, GPIO_PIN_1},
							{SYSCTL_PERIPH_GPION, OUTPUT, GPIO_PORTN_BASE, GPIO_PIN_0},
							{SYSCTL_PERIPH_GPIOF, OUTPUT, GPIO_PORTF_BASE, GPIO_PIN_4},
							{SYSCTL_PERIPH_GPIOF, OUTPUT, GPIO_PORTF_BASE, GPIO_PIN_0},
							};

/* BUTTONS */
udef_GPIO_Pin buttons[2] = {	{SYSCTL_PERIPH_GPIOJ, INPUT, GPIO_PORTJ_BASE, GPIO_PIN_0},
								{SYSCTL_PERIPH_GPIOJ, INPUT, GPIO_PORTJ_BASE, GPIO_PIN_1}
								};

/* ----------------------- FUNCTION PROTOTYPES ----------------------- */
void ISR_gpioUsrSW (void);								//Interrupt service Routine to Handle UsrSW Interrupt
void inithardware (void);								//Initialises Hardware (inputs/outputs)
void initinterrupts (void);								//Initialises Interrupts
void ms_delay (uint32_t ms);
void udef_GPIO_Pin_set_function (udef_GPIO_Pin *pins, uint8_t num_of_pins);
void GPIO_Pin_write (udef_GPIO_Pin *pin2set, uint32_t h_or_l);

/* ----------------------- FUNCTIONS ----------------------- */
int main(void) {
	int i = 0;
	inithardware();
	initinterrupts();
	IntMasterEnable();		//Enable Processor Interrupts

   for(i = 0; i < 4; i++){
	   GPIO_Pin_write(&leds[i], HIGH);
	   ms_delay(1000);
   }
   for(i = 3; i >= 0; i--){
	   GPIO_Pin_write(&leds[i], LOW);
	   ms_delay(1000);
   }
}

void ISR_gpioUsrSW(void) {
    if(GPIOIntStatus(buttons[0].port_base, false) & buttons[0].pin) {	//if USRSW1 pressed
        GPIOIntClear(buttons[0].port_base, buttons[0].pin);	    	//Clear the GPIO interrupt
    	//something happens
 	   GPIO_Pin_write(&leds[1], HIGH);
    }
    else {
        GPIOIntClear(buttons[1].port_base, buttons[1].pin);	    	//Clear the GPIO interrupt
    	//something happens
 	   GPIO_Pin_write(&leds[2], HIGH);
    }
}

void initinterrupts () {
	/* BUTTONS */
    GPIOIntRegister(buttons[0].port_base, ISR_gpioUsrSW);					//Register Handler for Port
    GPIOIntTypeSet(buttons[0].port_base, buttons[0].pin | buttons[1].pin, GPIO_RISING_EDGE);	//Set Interrupt-Type for Pin (Port, Pin, Type)
    GPIOIntClear(buttons[0].port_base, buttons[0].pin | buttons[1].pin);						//Clear possible interrupts
    GPIOIntEnable(buttons[0].port_base, buttons[0].pin | buttons[1].pin);					//Enables Interrupt from Port+Pin
}

void udef_GPIO_Pin_set_function(udef_GPIO_Pin *pins, uint8_t num_of_pins){
	int i = 0;
    uint32_t _ui32Strength;
    uint32_t _ui32PinType;

	for( i = 0; i < num_of_pins; i++){
		if(!SysCtlPeripheralReady(pins[i].peripheral)){
			SysCtlPeripheralEnable(pins[i].peripheral);
		}

		while(!SysCtlPeripheralReady(pins[i].peripheral))ms_delay(1);

		if(pins[i].pin_type == INPUT){
			GPIOPinTypeGPIOInput(pins[i].port_base, pins[i].pin);
		}else if(pins[i].pin_type == OUTPUT){
			GPIOPinTypeGPIOOutput(pins[i].port_base, pins[i].pin);
		}

		GPIOPadConfigGet(pins[i].port_base, pins[i].pin, &_ui32Strength, &_ui32PinType);
		GPIOPadConfigSet(pins[i].port_base, pins[i].pin, _ui32Strength, pins[i].pin_type);
	}
}

void inithardware () {
    gui32_SysClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, F_CPU);

    /* TIMER */
    SysTickPeriodSet(gui32_SysClock / TIMER_SCALE);

    /* BUTTONS */
    udef_GPIO_Pin_set_function(buttons, 2);

	/* LEDS */
    udef_GPIO_Pin_set_function(leds, 4);
}

void GPIO_Pin_write (udef_GPIO_Pin *pin2set, uint32_t h_or_l){
	GPIOPinWrite(pin2set->port_base, pin2set->pin, h_or_l);
}

void ms_delay(uint32_t ms){
	if(ms != 0){
		ms = (F_CPU/3000) * ms;
		SysCtlDelay(ms);
	}
}
