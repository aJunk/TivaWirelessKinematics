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
#define TIMER_SCALE_100MS 10		//triggers adc every 100 ms
#define TIMER_SCALE TIMER_SCALE_100MS

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
   IntMasterEnable();		//Enable Processor Interrupts

   addMove (FULL_STP, CW_TURN, CW_TURN, GEAR_CONV_FACTOR*16, 0, LOW);
   addMove (FULL_STP, CW_TURN, CW_TURN, 0, GEAR_CONV_FACTOR*16, LOW);
 //  addMove (FULL_STP, CCW_TURN, CCW_TURN, GEAR_CONV_FACTOR*128, GEAR_CONV_FACTOR*128, LOW);
 //  addMove (FULL_STP, CW_TURN, CCW_TURN, 64*16, 64*32, LOW);
 //  ms_delay(16000);
//   addMove (FULL_STP, CW_TURN, CW_TURN, 0, 64*8, LOW);
 //  addMove (FULL_STP, CCW_TURN, CCW_TURN, 64*16, 64*8, LOW);
   while(1);
}

void inithardware () {
    gui32_SysClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, F_CPU);

    /* TIMER */
    SysTickPeriodSet(gui32_SysClock / TIMER_SCALE);

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
	setMotorMode(&motor1, FULL_STP);
	setDirection(&motor1, CW_TURN);
	setMotorMode(&motor2, FULL_STP);
	setDirection(&motor2, CW_TURN);
}

void initinterrupts () {
	/* BUTTONS */
    GPIOIntRegister(buttons[0].port_base, ISR_gpioUsrSW);					//Register Handler for Port
    GPIOIntTypeSet(buttons[0].port_base, buttons[0].pin | buttons[1].pin, GPIO_RISING_EDGE);	//Set Interrupt-Type for Pin (Port, Pin, Type)
    GPIOIntClear(buttons[0].port_base, buttons[0].pin | buttons[1].pin);						//Clear possible interrupts
    GPIOIntEnable(buttons[0].port_base, buttons[0].pin | buttons[1].pin);					//Enables Interrupt from Port+Pin

    /* TIMER */
    SysTickPeriodSet(gui32_SysClock / 1000);    	//sets period of timer, ticks is number of clock ticks in a period -> hier: 100 mal pro sekunde
    SysTickIntRegister(ISR_SystickHandler);
    SysTickIntEnable();			//Enable Timer-Interrupts
    SysTickEnable();			//Start Timer
}

void ISR_gpioUsrSW(void) {
    if(GPIOIntStatus(buttons[0].port_base, false) & buttons[0].pin) {	//if USRSW1 pressed
        GPIOIntClear(buttons[0].port_base, buttons[0].pin);	    	//Clear the GPIO interrupt
    	//something happens
        addMove (FULL_STP, CW_TURN, CW_TURN, GEAR_CONV_FACTOR*32, 0, 0);
    }
    else {
        GPIOIntClear(buttons[1].port_base, buttons[1].pin);	    	//Clear the GPIO interrupt
    	//something happens
        addMove (HALF_STP, CCW_TURN, CCW_TURN, 0, GEAR_CONV_FACTOR*2, 1);
    }
}

void ISR_SystickHandler(void) {
	static uint8_t systickcounter = 0;
	static uint8_t systickcounterTotal = 0;
	static uint8_t accFactor1 = 6;		//how slow should acceloration start (higher = slower)
	static uint8_t accFactor2 = 6;		//how slow should acceloration start (higher = slower)
	static uint8_t accSpeed = 100;		//how fast should it become faster (lower = faster)
	/*static uint8_t sameDirFlag1 = 0;	//set if last motor direction == new motor direction -> no acceloration necessary
	static uint8_t sameDirFlag2 = 0;
	static uint8_t systickcounterTotalwhenFinished1 = 0;	//to safe number of systicks when motor stopped moving
	static uint8_t systickcounterTotalwhenFinished2 = 0;*/

	systickcounter++;
	systickcounterTotal++;

	if((systickcounterTotal % accSpeed) == 0 && accFactor1 != 2) accFactor1--;	//2 is final accFactor
	if (gui32_moveQ[gui32_actIdx2move].numMicroSteps1 > 0){
		if(systickcounter >= accFactor1)	{	//counter läuft 1000 mal pro sekunde über
			makeStep(&motor1);
			if (gui32_moveQ[gui32_actIdx2move].numMicroSteps2 == 0) systickcounter = 0; //otherwise motor2 would not move because systickcounter is set to 0
		}
	}
	if((systickcounterTotal % accSpeed) == 0 && accFactor2 != 2) accFactor2--;	//2 is final accFactor
	if (gui32_moveQ[gui32_actIdx2move].numMicroSteps2 > 0){
		if(systickcounter >= accFactor2)	{	//counter läuft 1000 mal pro sekunde über
			makeStep(&motor2);
			systickcounter = 0;
		}
	}
	if(gui32_moveQ[gui32_actIdx2move].numMicroSteps1 == 0 && gui32_moveQ[gui32_actIdx2move].numMicroSteps2 == 0) {
		accFactor1 = 6;
		accFactor2 = 6;
		systickcounterTotal = 0;
	}
	if(gui32_moveQ[gui32_actIdx2move].numMicroSteps1 == 0 && gui32_moveQ[gui32_actIdx2move].numMicroSteps2 == 0 && gui32_numMovesInQ != 0) changeActualMove();	//look for new moves in Q
}
