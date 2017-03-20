// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

#define PortB_Out	(*((volatile uint32_t *)0x4000503C))


// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
  SYSCTL_RCGC2_R |= 0x02;		// 1) enable clock to B without changing any other bits
  while((SYSCTL_RCGC2_R& 0xFF) == 0){};	// delay
  GPIO_PORTB_DIR_R |= 0x0F;		// Sets PB[0-3] to OUT
  GPIO_PORTB_AFSEL_R &= ~0x0F;		// Disable AFSEL on PB[0-3]
  GPIO_PORTB_DEN_R |= 0x0F;		// Enable DIGITAL on PB[0-3]
  
  PortB_Out = 0;
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Output: none
void DAC_Out(uint32_t data){
  PortB_Out =  data; 
}
