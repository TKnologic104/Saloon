// ADC.h
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0

// SysTick_Init

void SysTick_Init(void);

// 
void PortF_Init(void);

// ADC initialization slide and switch function 
// Input: none
// Output: none
void ADC_Init(void);

//---------------------ADC_In---------------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void);











