#include "functions.h"

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

/* ----------------------- FUNCTIONS ----------------------- */
void ms_delay(uint32_t ms){
	if(ms != 0){
		ms = (F_CPU/3000) * ms;
		SysCtlDelay(ms);
	}
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

void GPIO_Pin_write (udef_GPIO_Pin *pin2set, uint8_t h_or_l){
	GPIOPinWrite(pin2set->port_base, pin2set->pin, h_or_l);
}
