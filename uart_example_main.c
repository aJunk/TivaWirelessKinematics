/**
 * Obtain Processor core temperature via ADC and send it to PC over UART0
 * NOTE: Due to snprintf() the stack size must be set to 2048 bytes or higher.
 * Project Properties -> ARM Linker -> Basic Options -> "Set C System Stack Size"
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"

/* ----------------------- DEFINES ----------------------- */
/*ADC*/
#define HIGHEST_PRIORITY 0 			//priority of sequence
#define FIRST_STEP_IN_SEQUENCE 0 	//id of the step in the sequence
#define FIRST_SEQUENCE_ID 0 		//this adc can have several sequences, this is the id of the first sequence

/*TIMER*/
#define TIMER_SCALE_100MS 10		//triggers adc every 100 ms
#define TIMER_SCALE TIMER_SCALE_100MS
#define TIMER_RES 100				//timer resolution -> Time = Timer_Count * Timer_Res
#define TIMER_MAX_COUNT -1			//If -1 is assigned to an unsigned int this resolves to the biggest available number for this data type (two's complement)
#define TIMER_MIN_COUNT 2			//standard: 200 ms
#if TIMER_MIN_COUNT < 1
	#error "TIMER_MIN_COUNT must at least be 1"
#endif

/*BUTTONS*/
#define USRSW1 GPIO_PIN_0
#define USRSW2 GPIO_PIN_1
#define INT_USRSW1 GPIO_INT_PIN_0
#define INT_USRSW2 GPIO_INT_PIN_1

/*UART*/
#define TXBUF_LEN 128			//Size of send-buffer

/* ----------------------- GLOBAL VARIABLES ----------------------- */
/*TIMER*/
volatile uint32_t gui32_timeout = TIMER_MIN_COUNT;
volatile uint32_t gui32_SysClock = 0;

/*ADC*/
volatile uint32_t gui32send_flag = 0;			//Data available: 1; else: 0
volatile uint32_t gui32raw_adc = 0;				//Data from ADC - saved in global var

/* ----------------------- FUNCTION PROTOTYPES ----------------------- */
void ISR_SystickHandler(void);			//ISR for Systicks -> Counts overflows
void ISR_ButtonHandler(void);			//ISR for Buttons -> Shorter or longer intervals
void ISR_ADCHandler(void);				//ISR for ADC -> Get Data out of it
void init_hardware(void);
void init_interrupts(void);

void main(void) {
    char tx_buf[TXBUF_LEN] = {0};
    uint32_t i = 0;
    uint8_t fifo_full = 1;
    uint32_t integer;		//DO NOT INIT
    uint32_t fraction = 0;
    uint32_t raw_adc;		//DO NOT INIT
	uint32_t timeout = 0;
    double temperature = 0.0;

    init_hardware();
    init_interrupts();
    IntMasterEnable();

    while(1) {
        SysCtlSleep();		//put processor to sleep - by default peripherals are still operational

        if(gui32send_flag == 1) {	//if data available (Set in ADC ISR)
        	//Get global vars
        	timeout = gui32_timeout;
        	raw_adc = gui32raw_adc;

        	//Calculate temperature in degree celsius
        	temperature = 147.5 - ((75.0 * (3.3) * (double)raw_adc ) /4096.0);

            //Calculate integer and fraction part of floating point number as fixed point numbers
            integer = (uint32_t)temperature;
            temperature = (double)temperature - (double)integer;
            fraction = (uint32_t)(temperature * 100);

            //Make string to give to UART
            snprintf(tx_buf, TXBUF_LEN, "Intervall %u ms | Temperature: %u.%u°C\r\n", timeout * TIMER_RES, integer, fraction);

            i = 0;
            while(tx_buf[i] != '\0') {					//While there is stuff in tx_buf
            	if(UARTSpaceAvail(UART0_BASE)) {		//Determines if there is any space in the transmit FIFO
            		UARTCharPutNonBlocking(UART0_BASE, tx_buf[i]);
            		i++;
            	}
            	else {
            		/*toggle led if fifo is full*/
            		fifo_full ^= GPIO_PIN_0;
            		GPIOPinWrite (GPIO_PORTN_BASE, GPIO_PIN_0, fifo_full);
            	}
            }
            gui32send_flag = 0;		//Set to flag that print has taken place; Set in ADC ISR -> Data available
        }
    }
}

void ISR_SystickHandler(void) {
	static uint32_t counter = 0;		//Count number of systick-timer overflows

	if(counter < gui32_timeout) {
		counter++;
	}
	else {
		GPIOIntEnable(GPIO_PORTJ_BASE, USRSW1 | USRSW2);	//enable gpio interrupts again (Buttons) - debounce -> Disabled in ISR-ButtonHandler
		ADCProcessorTrigger(ADC0_BASE, FIRST_SEQUENCE_ID);	//Trigger the sample sequence (Trigger scharf stellen)
		counter = 0;					//reset counter
	}
}

void ISR_ButtonHandler(void) {
    uint32_t status = GPIOIntStatus(GPIO_PORTJ_BASE, true);

    if(status & INT_USRSW1) { 			/*shorter intervalls*/
        GPIOIntClear(GPIO_PORTJ_BASE, USRSW1);
        if(gui32_timeout <= (uint32_t)TIMER_MIN_COUNT)
        	gui32_timeout = (uint32_t)TIMER_MIN_COUNT;
        else
        	gui32_timeout--;
    }
    else if(status & INT_USRSW2) {		/*longer intervalls*/
        GPIOIntClear(GPIO_PORTJ_BASE, USRSW2);
        if(gui32_timeout > (uint32_t)(TIMER_MAX_COUNT-1))
        	gui32_timeout = (uint32_t)TIMER_MIN_COUNT;
        else
        	gui32_timeout++;
    }

    /*to prevent bouncing - enabled again in ISR_SystickHandler*/
    GPIOIntDisable(GPIO_PORTJ_BASE,USRSW1 | USRSW2);
}

void ISR_ADCHandler(void) {
	uint32_t adc_val;

	ADCIntClear(ADC0_BASE, FIRST_SEQUENCE_ID);
	ADCSequenceDataGet(ADC0_BASE, FIRST_SEQUENCE_ID, &adc_val);		//Get data from ADC

	gui32raw_adc = adc_val;			//Save adc-Data global
	gui32send_flag = 1;				//Set send_flag -> Data to send available
}

void init_hardware(void) {
    uint32_t oldpadconfig_strength;
    uint32_t oldpadconfig_type;

    gui32_SysClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);

    /* TIMER */
    SysTickPeriodSet(gui32_SysClock / TIMER_SCALE);

    /* LED */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    /* BUTTONS */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, USRSW1 | USRSW2);
    GPIOPadConfigGet(GPIO_PORTJ_BASE, USRSW1, &oldpadconfig_strength, &oldpadconfig_type);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, USRSW1, oldpadconfig_strength, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigGet(GPIO_PORTJ_BASE, USRSW2, &oldpadconfig_strength, &oldpadconfig_type);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, USRSW2, oldpadconfig_strength, GPIO_PIN_TYPE_STD_WPU);

    /*ADC*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, FIRST_STEP_IN_SEQUENCE, ADC_TRIGGER_PROCESSOR, HIGHEST_PRIORITY);	//Configures the trigger source and priority of a sample sequence
    ADCSequenceStepConfigure(ADC0_BASE, FIRST_SEQUENCE_ID, FIRST_STEP_IN_SEQUENCE,		//Configure a step of the sample sequencer
				ADC_CTL_IE 		/*issue interrupt upon completion*/ |
				ADC_CTL_END		/*end of sample sequence, ADC must be triggered afterwards*/ |
				ADC_CTL_TS		/*onchip temperatur sensors*/
				);
    ADCSequenceEnable(ADC0_BASE, FIRST_SEQUENCE_ID);

    /*UART*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);		//uart connected to PA0, PA1, which are connected to VCP
    GPIOPinConfigure(GPIO_PA0_U0RX | GPIO_PA1_U0TX);				//Alternate function for GPIO-Pin -> connect PA0 and PA1 as uart rx and tx
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);	    //set pins as uart controlled

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_SYSTEM);	    		//use system clock as clock source for baud rate generator
    UARTFlowControlSet(UART0_BASE, UART_FLOWCONTROL_NONE);			// no flow control
    UARTConfigSetExpClk(UART0_BASE, gui32_SysClock, 115200,			//configure uart with 8n1@115200
            UART_CONFIG_WLEN_8 |			//8 Bit
            UART_CONFIG_PAR_NONE |			//No Parity
            UART_CONFIG_STOP_ONE			//1 Stop-Bit
            );

    UARTFIFOEnable(UART0_BASE);    	//enable hardware fifos
    UARTEnable(UART0_BASE);		  	//enable uart
}

void init_interrupts(void) {
	/* BUTTONS */
    GPIOIntRegister(GPIO_PORTJ_BASE, ISR_ButtonHandler);
    GPIOIntTypeSet(GPIO_PORTJ_BASE, USRSW1 |USRSW2, GPIO_RISING_EDGE);
    GPIOIntClear(GPIO_PORTJ_BASE, USRSW1 | USRSW2);
    GPIOIntEnable(GPIO_PORTJ_BASE,USRSW1 | USRSW2);

    /* TIMER */
    SysTickIntRegister(ISR_SystickHandler);
    SysTickEnable();
    SysTickIntEnable();

    /* ADC */
    ADCIntRegister(ADC0_BASE, FIRST_SEQUENCE_ID, ISR_ADCHandler);
    ADCIntClear(ADC0_BASE, FIRST_SEQUENCE_ID);
    ADCIntEnable(ADC0_BASE, FIRST_SEQUENCE_ID);
}
