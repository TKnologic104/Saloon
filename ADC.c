// put implementations for functions, explain how it works
// put your names here, date

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "ADC.h"
#define PF2       (*((volatile uint32_t *)0x40025010))

// ADC initialization function 
// Input: none
// Output: none
void ADC_Init(void){ 
	int32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x11;      // 1) activate clock for Port A,E 
  while((SYSCTL_PRGPIO_R&0x10) == 0){};
	GPIO_PORTE_DIR_R &= ~0x06;      // 2) make PE0,2 input
  GPIO_PORTE_AFSEL_R |= 0x04;     // 3) (a) enable alternate fun on PE2
	GPIO_PORTE_AFSEL_R &= ~0x01;		//		(b) disable on PE0
  GPIO_PORTE_DEN_R &= ~0x04;      // 4) (a) disable digital I/O on PE2
	GPIO_PORTE_DEN_R	|= 0x01;			//		(b) enable digital on PE0
  GPIO_PORTE_AMSEL_R |= 0x04;     // 5) (a) enable analog fun on PE2
	GPIO_PORTE_AMSEL_R &= ~0x01;		//		(b) disable on PE0	
  SYSCTL_RCGCADC_R |= 0x01;       // 6) activate ADC0 
  delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;
  ADC0_PC_R = 0x01;               // 7) configure for 125K 
  ADC0_SSPRI_R = 0x0123;          // 8) Seq 3 is highest priority
  ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
  ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+ 1;  // 11) Ain1 (PE2)
  ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
  ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3

}

void PortF_Init(void){
	
// PortF clock has been enabled in ADC_Init()	
  SYSCTL_RCGCGPIO_R |= 0x20;  						// activate port F
	while((SYSCTL_PRGPIO_R&0x20)==0){};
  GPIO_PORTF_DIR_R |= 0x0E;							  // make PF1-3 output (PF2 built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x0E;						// disable alt funct on PF1-3
  GPIO_PORTF_DEN_R |= 0x0E;   						// enable digital I/O on PF1-3
																					// configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R
	&0xFFFF000F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;     						// disable analog functionality on PF
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void){  
	uint32_t value;
  ADC0_PSSI_R = 0x0008;            
  while((ADC0_RIS_R&0x08)==0){};   
  value = ADC0_SSFIFO3_R&0xFFF; 
  ADC0_ISC_R = 0x0008; 
  return value;
	}

void SysTick_Init(void){
	uint32_t delay = 2000000; 																				//40Hz sampling rate. 1/40 = 25ms ; 12.5ns is 1 cycle so 25ms = 2,000,000																																		
  NVIC_ST_CTRL_R = 0;																								// 1) Clear ENABLE bit.
  NVIC_ST_RELOAD_R = delay;																					// 2) Set RELOAD vector.
  NVIC_ST_CURRENT_R = 0;																						// 3) Clear CURRENT.
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF) | 0x40000000;			// Set priority to 2.
  NVIC_ST_CTRL_R |= 0x00000007;																			// 4) Set SRC (bit2 clock source is internal), INTEN (bit1 Interrupt Enable) 
}



