; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; Details: Implements Euclidean Division (Quotient-Remainder Theorem) through
; 		   recursion! 
;	Euclidean Division Theorem	
;		a = b*q + r,
;	where *a* is the sum of the quotient *q* and  divisor *b* plus the remainder *r*,
;	r = MOD[a/10] = A%10,
;	q = DIV[a/10] = 10|A;	
; In this code:
; 	R3 is the *divisor*, R2 is the *dividen*, 
;	R1 is the remainder, R0 is the quotient;
;
; WARNING: This code makes every effort not to implement a new stack frame! Thus,
;		   there are plenty of GOTO statements within. Proceed at your own risk.
;		   Also, I like abstract things way too much...

LCD_OutDec				; Set it up!! (This will all make sense in the future)
	MOV R1, LR			;	R0 = R1 = LR will be a special indication of 
	PUSH {R1, LR}		;	reaching the "bottom" of the stack.
	PUSH {R1, LR}
	MOV R3, #10			;	R3 is simply the *divisor*
	
Stack_It
	CMP R0, #10			; if (R0 < 10){
	BHS Not_Yet			;	LCD_OutChar(R0 + 0x30);
	ADD R0, R0, #48		;	GOTO Start_Popping; // hurray! 0x30 or #48 is the ascii offset
	BL  ST7735_OutChar	; }
	B Start_Popping		;
	
Not_Yet					; else { 
	MOV R2, R0			;	R2 := R0;
	UDIV R0, R2, R3		;	R0 := 10|R2; 
	MUL R1, R0, R3		;	R1 := 10*R0;
    SUB R1, R2, R1		; 	R1 := R2 - R1 = R2%10;
	PUSH {R0,R1}		;	GOTO Stack_It;
	PUSH {R1, LR}		;
	B Stack_It			; }
	
Start_Popping			; Yay! =')
	POP	{R1, LR}		;
	POP {R0, R1}		; while(R0 != R1){
	CMP R1, LR			;	R0 := R1;
	BEQ	Stop_Popping	;	LCD_OutChar(R0 + 0x30);
	ADD R0, R1, #48		;	R0 := Previous(R0);
	BL 	ST7735_OutChar	;	R1 := Previous(R1);
	B Start_Popping		; }
	  
Stop_Popping			; Aww... 
	MOV LR, R1	
	BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix				; Once more, set it up!!!!!!!
	MOV R1, LR			;	R1 = LR is a special indication of reaching
	PUSH {R1, LR}		;	the "bottom" of the stack.
	MOV R1, R0			; 	R1 is preserved for a constant.
	MOV R3, #3			; 	R3 is a counter of 3.
	MOV R0, #10000
	
	CMP R1, R0			; if(R1 >= 10,000){
	BLO	Stack_It_2		;	LCD_OutChar(0x2A);	
						;	LCD_OutChar(0x2E);
	MOV R0, #42			;	LCD_OutChar(0x2A);	
	BL ST7735_OutChar	;	LCD_OutChar(0x2A);	
	MOV R0, #46			;	LCD_OutChar(0x2A);	
	BL ST7735_OutChar	;	GOTO Stop_Popping_2;
	MOV R0, #42			;}
	BL ST7735_OutChar	;
	MOV R0, #42			;
	BL ST7735_OutChar	; else {
	MOV R0, #42			;	GOTO Stack_It_2;
	BL ST7735_OutChar	; }
	POP {R1, LR}		; 
	B Stop_Popping_2	;
	
Stack_It_2
	CMP R3, #0			; if (R3 == 0){
	BNE Not_Yet_2		;	LCD_OutChar(R0 + 0x30);
	ADD R0, R1, #48		;	LCD_OutChar((R0 = 0x2E));
	BL  ST7735_OutChar	; 	GOTO Start_Popping; // hurray again!
	MOV R0, #46			; }
	BL 	ST7735_OutChar	;
	B Start_Popping_2	;
	
Not_Yet_2				; else { 
	SUB R3, #1			;	R3--;						R3 is the counter
	MOV R2, R1			;	R2 == R1;
	MOV R0, #10			; 	R0 == 10;	
	UDIV R1, R2, R0		;	R1 := 10|R2; 				R1 is the quotient 
	MUL R0, R1, R0		;	R0 := 10*R1; R0 = bq		R0 is the remainder
    SUB R0, R2, R0		; 	R0 := R2 - R0 = R2%10;
	PUSH {R0, R1}		;	GOTO Stack_It;
	B Stack_It_2		; }
	
Start_Popping_2			; This was not as fun...
	POP {R0, R1}		; while(R3 != 3){		
	ADD R0, R0, #48		;	R0 = Previous(R0);
	BL 	ST7735_OutChar	;	R1 = Previous(R1);
	POP {R0, R1}		; }
	ADD R0, R0, #48		;	
	BL 	ST7735_OutChar	;	
	POP {R0, R1}		;
	ADD R0, R0, #48		;	
	BL 	ST7735_OutChar	;
	POP {R1, LR}
	  
Stop_Popping_2			; Aww... 
	BX  LR

     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
