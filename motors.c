#include "motors.h"

/* ----------------------- GLOBAL VARIABLES ----------------------- */
/*motors motor1 = {	{SYSCTL_PERIPH_GPIOD, OUTPUT, GPIO_PORTD_BASE, GPIO_PIN_2},
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
					CW_TURN, FULL_STP, 0};*/

motors motor1 = {	{SYSCTL_PERIPH_GPIOE, OUTPUT, GPIO_PORTE_BASE, GPIO_PIN_0},
					{SYSCTL_PERIPH_GPIOE, OUTPUT, GPIO_PORTE_BASE, GPIO_PIN_1},
					{SYSCTL_PERIPH_GPIOE, OUTPUT, GPIO_PORTE_BASE, GPIO_PIN_2},
					{SYSCTL_PERIPH_GPIOE, OUTPUT, GPIO_PORTE_BASE, GPIO_PIN_3},
					{SYSCTL_PERIPH_GPIOE, OUTPUT, GPIO_PORTE_BASE, GPIO_PIN_4},
					CW_TURN, FULL_STP, 0};

motors motor2 = {	{SYSCTL_PERIPH_GPIOD, OUTPUT, GPIO_PORTD_BASE, GPIO_PIN_7},
					{SYSCTL_PERIPH_GPIOA, OUTPUT, GPIO_PORTA_BASE, GPIO_PIN_6},
					{SYSCTL_PERIPH_GPIOM, OUTPUT, GPIO_PORTM_BASE, GPIO_PIN_4},
					{SYSCTL_PERIPH_GPIOM, OUTPUT, GPIO_PORTM_BASE, GPIO_PIN_5},
					{SYSCTL_PERIPH_GPIOC, OUTPUT, GPIO_PORTC_BASE, GPIO_PIN_4},
					CW_TURN, FULL_STP, 0};

volatile move gui32_moveQ[MAX_NUM_MOVES];
volatile uint32_t gui32_actIdx2move = 0;
volatile uint32_t gui32_actIdx2add = 0;
volatile uint32_t gui32_numMovesInQ = 0;
volatile uint8_t gui32_actualInMove = 0;
volatile float angleM1 = 0;
volatile float angleM2 = 0;

/* ----------------------- FUNCTIONS ----------------------- */
void setMotorMode (motors *motor, uint32_t mode){
	motor->mode = mode;			//set mode in global var
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

uint8_t checkIfQisFull () {
	if(gui32_numMovesInQ == MAX_NUM_MOVES){
		GPIO_Pin_write(&(leds[0]), HIGH);
		return 1;
	} else GPIO_Pin_write(&(leds[0]), LOW);
	return 0;
}

uint8_t addMove (uint32_t mode, uint32_t direction1, uint32_t direction2, uint32_t numSteps1, uint32_t numSteps2, uint32_t numDoAgain) {
	/* Exit if queue is already full */
	if(checkIfQisFull()) return 0;

	/* Write move-information to move-queue */
	gui32_moveQ[gui32_actIdx2add].mode = mode;
	gui32_moveQ[gui32_actIdx2add].direction[0] = direction1;
	gui32_moveQ[gui32_actIdx2add].direction[1] = direction2;
	gui32_moveQ[gui32_actIdx2add].numSteps[0]= numSteps1;
	gui32_moveQ[gui32_actIdx2add].numSteps[1] = numSteps2;
	gui32_moveQ[gui32_actIdx2add].constNumSteps[0] = numSteps1;
	gui32_moveQ[gui32_actIdx2add].constNumSteps[1] = numSteps2;
	gui32_moveQ[gui32_actIdx2add].numDoAgain = numDoAgain;
	gui32_numMovesInQ++;

	/* If there were no moves in queue, the added move has to be done */
	if(gui32_numMovesInQ == 1) setActualParameters ();

	/* Recalculate position of ring-buffer-write-pointer */
	if(gui32_actIdx2add == MAX_NUM_MOVES-1) gui32_actIdx2add = 0;		//if end of queue -> start over again
	else gui32_actIdx2add++;

	return 1;
}

void changeActualMove (){
	gui32_numMovesInQ--;

	/* Put move in queue again if it should be done again */
	if(gui32_moveQ[gui32_actIdx2move].numDoAgain != 0){
		gui32_moveQ[gui32_actIdx2move].numDoAgain--;
        addMove (gui32_moveQ[gui32_actIdx2move].mode, gui32_moveQ[gui32_actIdx2move].direction[0], gui32_moveQ[gui32_actIdx2move].direction[1], gui32_moveQ[gui32_actIdx2move].constNumSteps[0], gui32_moveQ[gui32_actIdx2move].constNumSteps[1], gui32_moveQ[gui32_actIdx2move].numDoAgain);
	}

	/* Recalculate position of ring-buffer-read-pointer */
	if(gui32_actIdx2move == MAX_NUM_MOVES-1) gui32_actIdx2move = 0;		//if end of queue -> start over again
	else gui32_actIdx2move++;
	setActualParameters();
	checkIfQisFull ();
}

void setActualParameters () {
	setDirection(&motor1, gui32_moveQ[gui32_actIdx2move].direction[0]);
	setMotorMode(&motor1, gui32_moveQ[gui32_actIdx2move].mode);
	setDirection(&motor2, gui32_moveQ[gui32_actIdx2move].direction[1]);
	setMotorMode(&motor2, gui32_moveQ[gui32_actIdx2move].mode);
}

void makeStep (motors *motor){
	/* Set motor pin to make a step */
	gui32_actualInMove = 1;
	if (GPIOPinRead(motor->stp.port_base, motor->stp.pin) & motor->stp.pin) {
		GPIO_Pin_write(&(motor->stp), LOW);
	}
	else {
		GPIO_Pin_write(&(motor->stp), HIGH);

		/* Recalculate actual number of microsteps done for the gloable motor-var */
		if (motor->direction == CW_TURN) motor->numTotalMicroSteps = motor->numTotalMicroSteps + MICRO_STP/motor->mode;	//1 for MICRO_STP, 2 for HALF_STP, etc.
		else motor->numTotalMicroSteps = motor->numTotalMicroSteps - MICRO_STP/motor->mode;
		calcAngles();

		/* Recalculate number of steps of moves which have to be done */
		if(motor == &motor1) {
			gui32_moveQ[gui32_actIdx2move].numSteps[0]--;
		} else {
			gui32_moveQ[gui32_actIdx2move].numSteps[1]--;
		}

		/* If both motors have finished move go to next move / wait */
		if(gui32_moveQ[gui32_actIdx2move].numSteps[0] == 0 && gui32_moveQ[gui32_actIdx2move].numSteps[1] == 0) {
			gui32_actualInMove = 0;
			changeActualMove();
		}
	}
}


void calcAngles(void){
	angleM1 = (motor1.numTotalMicroSteps)*360 / ((MICRO_STP*GEAR_CONV_FACTOR));
	angleM2 = (motor2.numTotalMicroSteps)*360 / ((MICRO_STP*GEAR_CONV_FACTOR));
}
