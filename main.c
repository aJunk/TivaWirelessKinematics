#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "functions.h"
#include "motors.h"

/* ----------------------- DEFINES ----------------------- */
/* GENERAL */
#define NULL 0

/* TIMER */
#define TIMER_SCALE 1000

/* ----------------------- GLOBAL VARIABLES ----------------------- */
/* TIMER */
volatile uint32_t gui32_SysClock = 0;

/* ----------------------- FUNCTION PROTOTYPES ----------------------- */
void ISR_gpioUsrSW (void);								//Interrupt service Routine to Handle UsrSW Interrupt
void ISR_SystickHandler(void);							//Interrupt service Routine
void inithardware (void);								//Initialises Hardware (inputs/outputs)
void initinterrupts (void);								//Initialises Interrupts

/* ----------------------- FUNCTIONS ----------------------- */
int main(void) {
   inithardware();
   initinterrupts();
   IntMasterEnable();

   addMove (FULL_STP, CW_TURN, CW_TURN, GEAR_CONV_FACTOR*16, 0, 0);
   addMove (FULL_STP, CW_TURN, CW_TURN, 0, GEAR_CONV_FACTOR*16, 0);
   while(1);
}

void inithardware () {
    gui32_SysClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, F_CPU);

    /* BUTTONS */
    udef_GPIO_Pin_set_function(buttons, 2);

	/* LEDS */
    udef_GPIO_Pin_set_function(leds, 4);

    /* MOTOR */
    udef_GPIO_Pin_set_function(&motor1.ms1, 1);
    udef_GPIO_Pin_set_function(&motor1.ms2, 1);
    udef_GPIO_Pin_set_function(&motor1.dir, 1);
    udef_GPIO_Pin_set_function(&motor1.stp, 1);
    udef_GPIO_Pin_set_function(&motor1.en, 1);
    udef_GPIO_Pin_set_function(&motor2.ms1, 1);
    udef_GPIO_Pin_set_function(&motor2.ms2, 1);
    udef_GPIO_Pin_set_function(&motor2.dir, 1);
    udef_GPIO_Pin_set_function(&motor2.stp, 1);
    udef_GPIO_Pin_set_function(&motor2.en, 1);
	GPIO_Pin_write(&motor1.en, LOW);
	GPIO_Pin_write(&motor2.en, LOW);
}

void initinterrupts () {
	/* BUTTONS */
    GPIOIntRegister(buttons[0].port_base, ISR_gpioUsrSW);
    GPIOIntTypeSet(buttons[0].port_base, buttons[0].pin | buttons[1].pin, GPIO_RISING_EDGE);
    GPIOIntClear(buttons[0].port_base, buttons[0].pin | buttons[1].pin);
    GPIOIntEnable(buttons[0].port_base, buttons[0].pin | buttons[1].pin);

    /* TIMER */
    SysTickPeriodSet(gui32_SysClock / TIMER_SCALE);    	//sets period of timer -> overflow 1000x per second
    SysTickIntRegister(ISR_SystickHandler);
    SysTickIntEnable();
    SysTickEnable();
}

void ISR_gpioUsrSW(void) {
    if(GPIOIntStatus(buttons[0].port_base, false) & buttons[0].pin) {	//if USRSW1 pressed
        GPIOIntClear(buttons[0].port_base, buttons[0].pin);
        addMove (FULL_STP, CW_TURN, CW_TURN, GEAR_CONV_FACTOR*8, 0, 0);
        ms_delay(20);	//debounce
    }
    else {
        GPIOIntClear(buttons[1].port_base, buttons[1].pin);
        addMove (FULL_STP, CCW_TURN, CCW_TURN, 0, GEAR_CONV_FACTOR*8, 1);
        ms_delay(20);
    }
}

void ISR_SystickHandler(void) {
	static uint8_t systickcounter = 0;
	static uint8_t systickcounterTotal = 0;
	static uint8_t accFactor1 = 6;		//how slow should acceloration start (higher = slower)
	static uint8_t accFactor2 = 6;		//how slow should acceloration start (higher = slower)
	static uint8_t accSpeed = 100;		//how fast should it become faster (lower = faster)

	systickcounter++;
	systickcounterTotal++;

	if((systickcounterTotal % accSpeed) == 0 && accFactor1 != 2) accFactor1--;	//2 is final accFactor
	if (gui32_moveQ[gui32_actIdx2move].numSteps[0] > 0){
		if(systickcounter >= accFactor1)	{	//counter läuft 1000 mal pro sekunde über
			makeStep(&motor1);
			if (gui32_moveQ[gui32_actIdx2move].numSteps[1] == 0) systickcounter = 0; //otherwise motor2 would not move because systickcounter is set to 0
		}
	}
	if((systickcounterTotal % accSpeed) == 0 && accFactor2 != 2) accFactor2--;	//2 is final accFactor
	if (gui32_moveQ[gui32_actIdx2move].numSteps[1] > 0){
		if(systickcounter >= accFactor2)	{	//counter läuft 1000 mal pro sekunde über
			makeStep(&motor2);
			systickcounter = 0;
		}
	}
	if(gui32_moveQ[gui32_actIdx2move].numSteps[0] == 0 && gui32_moveQ[gui32_actIdx2move].numSteps[1] == 0) {
		accFactor1 = 6;
		accFactor2 = 6;
		systickcounterTotal = 0;
	}
}
