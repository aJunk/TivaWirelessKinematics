#include "motors.h"

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

volatile move gui32_moveQ[MAX_NUM_MOVES];
volatile uint32_t gui32_actIdx2move = 0;
volatile uint32_t gui32_actIdx2add = 0;
volatile float angleM1 = 0;
volatile float angleM2 = 0;

/* ----------------------- FUNCTIONS ----------------------- */
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

void addMove (uint32_t mode, uint32_t direction1, uint32_t direction2, uint32_t numMicroSteps1, uint32_t numMicroSteps2, uint32_t doAgainFlag) {
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
		return;
	}

	gui32_moveQ[gui32_actIdx2add].mode = mode;
	gui32_moveQ[gui32_actIdx2add].direction1 = direction1;
	gui32_moveQ[gui32_actIdx2add].direction2 = direction2;
	gui32_moveQ[gui32_actIdx2add].numMicroSteps1 = numMicroSteps1;
	gui32_moveQ[gui32_actIdx2add].numMicroSteps2 = numMicroSteps2;
	gui32_moveQ[gui32_actIdx2add].doAgainFlag = doAgainFlag;
	return;
}
