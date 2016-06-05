#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "functions.h"

#ifndef MOTORS_H
#define MOTORS_H

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
	uint32_t mode;		//to drive in
	uint32_t direction1;	//to drive
	uint32_t direction2;	//to drive
	uint32_t numMicroSteps1;	//to drive
	uint32_t numMicroSteps2;
	uint32_t numDoAgain;
}move;

/* ----------------------- DEFINES ----------------------- */
/* MOTORS */	//For 1 turn: 1/1: 32; 1/2: 64; 1/4: 128; 1/8: 256
#define FULL_STP 32
#define HALF_STP 64
#define QUARTER_STP 128
#define MICRO_STP 256
#define GEAR_CONV_FACTOR 64		//-> FULL_STP*GEAR_CONV_FACTOR -> 1 rotation
#define CCW_TURN HIGH
#define CW_TURN LOW
#define MAX_NUM_MOVES 10

/* ----------------------- GLOBAL VARIABLES ----------------------- */
/* MOTORS */
extern motors motor1;
extern motors motor2;
extern volatile move gui32_moveQ[MAX_NUM_MOVES];
extern volatile uint32_t gui32_actIdx2move;
extern volatile uint32_t gui32_actIdx2add;
extern volatile uint32_t gui32_numMovesInQ;
extern volatile float angleM1;
extern volatile float angleM2;

/* ----------------------- FUNCTION PROTOTYPES ----------------------- */
void makeStep (motors *motor);						//sets pins of given motor to make a step
void setMotorMode (motors *motor, uint32_t mode);	//sets step-mode of given motor
void setDirection (motors *motor, uint32_t dir);	//sets direction of given motor
void addMove (uint32_t mode, uint32_t direction1, uint32_t direction2, uint32_t numMicroSteps1, uint32_t numMicroSteps2, uint32_t numDoAgain);	//add a move to queue
void calcAngles(void);								//calc actual motor position in deg (CW = +)
void changeActualMove ();
void setActualParameters ();
uint8_t checkIfQisFull ();

#endif
