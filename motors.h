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
	uint32_t mode;
	uint32_t direction[2];
	uint32_t numSteps[2];
	uint32_t constNumSteps[2];
	uint32_t numDoAgain;
}move;

/* ----------------------- DEFINES ----------------------- */
#define FULL_STP 32				//For 1 turn: 1/1: 32; 1/2: 64; 1/4: 128; 1/8: 256
#define HALF_STP 64
#define QUARTER_STP 128
#define MICRO_STP 256
#define GEAR_CONV_FACTOR 64		//-> FULL_STP*GEAR_CONV_FACTOR -> 1 rotation
#define CCW_TURN HIGH
#define CW_TURN LOW
#define MAX_NUM_MOVES 10

/* ----------------------- GLOBAL VARIABLES ----------------------- */
extern motors motor1;
extern motors motor2;
extern volatile move gui32_moveQ[MAX_NUM_MOVES];
extern volatile uint32_t gui32_actIdx2move;
extern volatile uint32_t gui32_actIdx2add;
extern volatile uint32_t gui32_numMovesInQ;
extern volatile uint8_t gui32_actualInMove;
extern volatile float angleM1;
extern volatile float angleM2;

/* ----------------------- FUNCTION PROTOTYPES ----------------------- */
void setMotorMode (motors *motor, uint32_t mode);
/* Sets appropriate pins of given motor to drive in given mode and sets mode-variable in global motor-variable
 * Parameters:		motors *motor 	is the pointer to the motor of which the mode should be changed
 * 					uint32_t mode 	is the mode the motor should be set to. For possibilities see defines
 * Return value: none
 */

void setDirection (motors *motor, uint32_t dir);
/* Sets appropriate pins of given motor to drive in given direction and sets direction-variable in global motor-variable
 * Parameters:		motors *motor 	is the pointer to the motor of which the direction should be changed
 * 					uint32_t dir 	is the direction the motor should be set to. For possibilities see defines
 * Return value: none
 */

uint8_t checkIfQisFull ();
/* Sets a led if the move-queue is full
 * Parameters: none
 * Return value: 	1	if queue is full
 * 					0	if there is at least 1 free space in queue
 */

uint8_t addMove (uint32_t mode, uint32_t direction1, uint32_t direction2, uint32_t numSteps1, uint32_t numSteps2, uint32_t numDoAgain);
/* Adds a move to the move-queue if queue is not full. Recalculates position of ring-buffer-write-pointer (gui32_actIdx2add).
 * Sets actual move-parameters if there were no moves in the queue.
 * Parameters:	uint32_t mode			is the mode to make the move in
 * 				uint32_t direction1		is the direction of motor1 to make the move in
 * 				uint32_t direction2		is the direction of motor2 to make the move in
 * 				uint32_t numSteps1		is the number of steps motor1 should make
 * 				uint32_t numSteps2		is the number of steps motor2 should make
 * 				uint32_t numDoAgain		if != 0 the move is put in move-queue again after move is done
 * Return value: 	1	if move is added to queue
 * 					0	if move could not be added to queue
 */

void changeActualMove (void);
/* Is called if a move is finished. Recalculates position of ring-buffer-read-pointer (which move is the next to be done).
 * Decreases number of moves in queue. Puts move in queue again if it should be done again (numDoAgain != 0)
 * Parameters: none
 * Return value: none
 */

void setActualParameters (void);
/* Is called if a move new move should be done. Calls setMotorMode and setDirection to set appropriate pins of motor1 and motor2
 * Parameters: none
 * Return value: none
 */

void makeStep (motors *motor);
/* Sets appropriate pin of make a step with the given motor. Calculates number of steps done with given motor and stores it in
 * global motor-variable. Calculates number of steps to do left in actual move. Calls changeActualMove() if both motors have
 * finished moving.
 * Parameters:		motors *motor 	is the pointer to the motor with which a step should be done
 * Return value: none
 */

void calcAngles (void);
/* Recalculates angle of motors done since init to visually keep control of position. Positive direction is clockwise.
 * Parameters: none
 * Return value: none
 */

#endif
