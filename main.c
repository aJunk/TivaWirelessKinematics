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

typedef struct motors {
	udef_GPIO_Pin ms1;
	udef_GPIO_Pin ms2;
	udef_GPIO_Pin dir;
	udef_GPIO_Pin stp;
	udef_GPIO_Pin en;
	uint32_t direction;	//actual set direction
	uint32_t mode;		//actual set mode - For 1 turn: 1/1: 32; 1/2: 64; 1/4: 128; 1/8: 256
	int32_t numTotalMicroSteps;	// in cw direction
}motors;

typedef struct move {
	//motors *motor;
	uint32_t mode;		//to drive in
	uint32_t direction1;	//to drive
	uint32_t direction2;	//to drive
	uint32_t numMicroSteps1;	//to drive
	uint32_t numMicroSteps2;
	uint32_t doAgainFlag;
	//struct move *nextMove;
}move;

/* ----------------------- DEFINES ----------------------- */
/* GENERAL */
#define NULL 0

/* MOTORS */	//For 1 turn: 1/1: 32; 1/2: 64; 1/4: 128; 1/8: 256
#define FULL_STP 32
#define HALF_STP 64
#define QUARTER_STP 128
#define MICRO_STP 256
#define GEAR_CONV_FACTOR 64		//-> FULL_STP*GEAR_CONV_FACTOR -> 1 rotation
#define CCW_TURN HIGH
#define CW_TURN LOW
#define MAX_NUM_MOVES 10

/* TIMER */
#define TIMER_SCALE_100MS 10		//triggers adc every 100 ms
#define TIMER_SCALE TIMER_SCALE_100MS

/* ----------------------- GLOBAL VARIABLES ----------------------- */
/* MOTORS */
motors motor1 = {	{SYSCTL_PERIPH_GPIOD, OUTPUT, GPIO_PORTD_BASE, GPIO_PIN_2},
					{SYSCTL_PERIPH_GPIOP, OUTPUT, GPIO_PORTP_BASE, GPIO_PIN_4},
					{SYSCTL_PERIPH_GPIOP, OUTPUT, GPIO_PORTP_BASE, GPIO_PIN_5},
					{SYSCTL_PERIPH_GPIOM, OUTPUT, GPIO_PORTM_BASE, GPIO_PIN_7},
					{SYSCTL_PERIPH_GPIOD, OUTPUT, GPIO_PORTD_BASE, GPIO_PIN_4},
					CW_TURN, FULL_STP, 0};

motors motor2 = {	{SYSCTL_PERIPH_GPIOE, OUTPUT, GPIO_PORTE_BASE, GPIO_PIN_4},
					{SYSCTL_PERIPH_GPIOC, OUTPUT, GPIO_PORTC_BASE, GPIO_PIN_7},
					{SYSCTL_PERIPH_GPIOH, OUTPUT, GPIO_PORTH_BASE, GPIO_PIN_2},
					{SYSCTL_PERIPH_GPIOM, OUTPUT, GPIO_PORTM_BASE, GPIO_PIN_3},
					{SYSCTL_PERIPH_GPIOC, OUTPUT, GPIO_PORTC_BASE, GPIO_PIN_6},
					CW_TURN, FULL_STP, 0};

/* TIMER */
volatile uint32_t gui32_SysClock = 0;

/* MOTORS */
volatile move gui32_moveQ[MAX_NUM_MOVES];
volatile uint32_t gui32_actIdx2move = 0;
volatile uint32_t gui32_actIdx2add = 0;
volatile float angleM1 = 0;
volatile float angleM2 = 0;

/* ----------------------- FUNCTION PROTOTYPES ----------------------- */
void ISR_gpioUsrSW (void);								//Interrupt service Routine to Handle UsrSW Interrupt
void ISR_SystickHandler(void);							//Interrupt service Routine
void inithardware (void);								//Initialises Hardware (inputs/outputs)
void initinterrupts (void);								//Initialises Interrupts
void makeStep (motors *motor);
void setMotorMode (motors *motor, uint32_t mode);
void setDirection (motors *motor, uint32_t dir);
int addMove (uint32_t mode, uint32_t direction1, uint32_t direction2, uint32_t numMicroSteps1, uint32_t numMicroSteps2, uint32_t doAgainFlag);
void calcAngles(void);

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

void ISR_gpioUsrSW(void) {
    if(GPIOIntStatus(buttons[0].port_base, false) & buttons[0].pin) {	//if USRSW1 pressed
        GPIOIntClear(buttons[0].port_base, buttons[0].pin);	    	//Clear the GPIO interrupt
    	//something happens
        addMove (FULL_STP, CW_TURN, CW_TURN, GEAR_CONV_FACTOR*32, 0, LOW);
    }
    else {
        GPIOIntClear(buttons[1].port_base, buttons[1].pin);	    	//Clear the GPIO interrupt
    	//something happens
        addMove (HALF_STP, CCW_TURN, CCW_TURN, 0, GEAR_CONV_FACTOR*1, LOW);
    }
}

void setMotorMode (motors *motor, uint32_t mode){
	motor->mode = mode;			//set mode in global var motor
	switch(mode) {
		case FULL_STP:
			GPIO_Pin_write(&(motor->ms1), LOW);
			GPIO_Pin_write(&(motor->ms2), LOW);
			break;
		case HALF_STP:
			GPIO_Pin_write(&(motor->ms1), HIGH);
			GPIO_Pin_write(&(motor->ms2), LOW);
			break;
		case QUARTER_STP:
			GPIO_Pin_write(&(motor->ms1), LOW);
			GPIO_Pin_write(&(motor->ms2), HIGH);
			break;
		case MICRO_STP:
			GPIO_Pin_write(&(motor->ms1), HIGH);
			GPIO_Pin_write(&(motor->ms2), HIGH);
			break;
	}
}

void setDirection (motors *motor, uint32_t dir){
	motor->direction = dir;
	GPIO_Pin_write(&(motor->dir), dir);	//dir=LOW -> CW or HIGH -> CCW
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

void calcAngles(void){
	angleM1 = (motor1.numTotalMicroSteps)*360 / ((MICRO_STP*GEAR_CONV_FACTOR));
	angleM2 = (motor2.numTotalMicroSteps)*360 / ((MICRO_STP*GEAR_CONV_FACTOR));
}

void makeStep (motors *motor){
	if (GPIOPinRead(motor->stp.port_base, motor->stp.pin) & motor->stp.pin) {
		GPIO_Pin_write(&(motor->stp), LOW);
	}
	else {
		GPIO_Pin_write(&(motor->stp), HIGH);
		if (motor->direction == CW_TURN) motor->numTotalMicroSteps = motor->numTotalMicroSteps + MICRO_STP/motor->mode;	//1 for MICRO_STP, 2 for HALF_STP, etc.
		else motor->numTotalMicroSteps = motor->numTotalMicroSteps - MICRO_STP/motor->mode;
		if(motor == &motor1) {
			gui32_moveQ[gui32_actIdx2move].numMicroSteps1--;
		} else {
			gui32_moveQ[gui32_actIdx2move].numMicroSteps2--;
		}
		calcAngles();
	}
}

int addMove (uint32_t mode, uint32_t direction1, uint32_t direction2, uint32_t numMicroSteps1, uint32_t numMicroSteps2, uint32_t doAgainFlag) {
	uint8_t noFreeSpace = 1;	//set to 0 if there is a free space in the queue
	uint8_t i = 0;

	for(i = 0; i < MAX_NUM_MOVES; i++) {
		if(gui32_moveQ[i].numMicroSteps1 == 0 && gui32_moveQ[i].numMicroSteps2 == 0){
			gui32_actIdx2add = i;
			noFreeSpace = 0;
			GPIO_Pin_write(&(leds[0]), LOW);
			break;
		}
	}
	if(noFreeSpace == 1) {
		GPIO_Pin_write(&(leds[0]), HIGH);
		return 1;
	}

	gui32_moveQ[gui32_actIdx2add].mode = mode;
	gui32_moveQ[gui32_actIdx2add].direction1 = direction1;
	gui32_moveQ[gui32_actIdx2add].direction2 = direction2;
	gui32_moveQ[gui32_actIdx2add].numMicroSteps1 = numMicroSteps1;
	gui32_moveQ[gui32_actIdx2add].numMicroSteps2 = numMicroSteps2;
	gui32_moveQ[gui32_actIdx2add].doAgainFlag = doAgainFlag;
	return 0;
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
			if (gui32_moveQ[gui32_actIdx2move].numMicroSteps2 == 0) systickcounter = 0; //otherwise motor2 would not move becuase systickcounter is set to 0
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

		if(gui32_actIdx2move == MAX_NUM_MOVES-1) gui32_actIdx2move = 0;		//if end of queue -> start over again
		else gui32_actIdx2move++;
		setDirection(&motor1, gui32_moveQ[gui32_actIdx2move].direction1);
		setMotorMode(&motor1, gui32_moveQ[gui32_actIdx2move].mode);
		setDirection(&motor2, gui32_moveQ[gui32_actIdx2move].direction2);
		setMotorMode(&motor2, gui32_moveQ[gui32_actIdx2move].mode);
	}
}

/* TODO: Funktion die gerufen wird und mode/direction etc. setzt wenn neuer move abgearbeitet wird*/
