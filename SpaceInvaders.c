// SpaceInvaders.c
// This is a starter project for the EE319K Lab 10

// ******************Hardware I/O Connections*********************
// __________Port A______________
//	LCD
// __________Port B______________
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)

//____________Port E______________
// ADC
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to a (+5V) voltage divider

// Switches
// Select button connected to PE0

// ____________Port F______________
// Heartbeats 

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "Images.h"

#define Heartbeat_Red	  	     (*((volatile uint32_t *)0x40025008))
#define Heartbeat_Blue	       (*((volatile uint32_t *)0x40025010))
#define PF3      							 (*((volatile uint32_t *)0x40025020))
#define PE0										 (*((volatile uint32_t *)0x40024004))
#define J_Objects								50

void DisableInterrupts(void); 		// Disable interrupts
void EnableInterrupts(void);  		// Enable interrupts
void Delay100ms(uint32_t count); 	// time delay in 0.1 seconds

//--------------------ADC, Slide, & Switch Variables------------------
uint32_t Slide_Value;
uint32_t Select_Value;
uint32_t ADCStatus = 0;

//--------------------ADC, Slide, & Switch Functions------------------


//--------------------Objects and their Pointers/Structures-----------
	
	//------------------Objects and their Properties
	//	Properties{									//Imported from header	
	//			pointer
	//			int16_t 	Height_n;
	//			int16_t 	Width_n;
	//			}	
	//	Remarks:	*n* is the name
	
	struct Properties {
		int8_t number;
		int16_t	height;
		int16_t width;
	};
	typedef struct Properties Prop;
	
	Prop TNT = {1, 15, 29};
	Prop Piano = {2, 10, 25};
	Prop Whiskey = {3, 11, 24};
	Prop Coffee = {4, 15, 29};

	//------------------Database of Objects
	// database {
	//			On or Off 
	//			Object and Properties
	//			Position_n = Select{x, y}
	//			
	//		}
	//	
	//	Remarks:	*i* may change based on the number of objects we may choose to incorperate. 
	//						Plese update the database accordingly.
	
	struct Data{
		int8_t toggle;
		Prop object;
		int16_t position_x;
		int16_t position_y;
	};
	typedef struct Data Database;
	//------------------Array of Objects
	//	Remarks:	An array of *j* Objects that could *possibly* be drawn on 
	//						the screen. Size *j* will be decided latter.
	// 
	//						j = 50; Declared at the begining. 	
	Database Objects[J_Objects];

	
	//-------------------Object Functions
void Clear_Objects(void){
		for(int i = 0; i<= 50; i++){		//Disable all the objects
		Objects[i].toggle = 0;
	}
}

	//-------------------New Object
	// Returns: 1 to increment the index or 0 otherwise.
int16_t New_Object(int16_t i){
	if(i == -1){
		return 0;
		}
	else
		return 1;
}
	//------------------Object Drop
	//	Returns: One of 3 objects with a progressive biase for the first one.
	//	Remarks:	
Prop Object_Drop(int16_t i){
	switch(i){
		case '0':
			return TNT;
		case '1':
			return Whiskey;
		case '2':
			return Piano;
		case	'3':
			return Coffee;
		default:
			return TNT;
	}
}

//--------------------Random_Functions----------------------------------

	//--------------------Random_Inverter
	// Returns: P(X==-1) == P(X==1) = 50% 
int16_t Random_Inverter(){
		if((Random()%2) == 1)
			return 1;
		else
			return -1;
	}

	//--------------------Random_Object	
	// Returns: P(X==0) = 60%
	//					P(X==1) = 15% 	
	//					P(X==2) = 15% 	
	//					P(X==3) = 10% 		
int16_t Random_Object(){
		int32_t i;
		
		if(i < 153){							
				return 0;
		}
		if((i >= 153) && (i < 191)){
				return 1;
		}
		if((i >= 191) && (i < 228)){
				return 2;
		}
		else{
				return 3;
		}
	}



	

//-----------------------Flags & Functions------------------------------
int8_t Flag_Drunk;
int8_t Flag_Lag;
int8_t Flag_Fast;

	// Milestones[i] = {Perfection, Dedicated, Hangover}
	// Inventory[i] = {Lives, Caffeine}
	// Remarks: Maintains the overall game conditions by boolean conditons
int8_t Milestones[3] = {1,1,0};
int8_t Inventory[2] = {3, 0};

	//-----------------------get_Boom()--------------------------------------
	// Remarks: Updates the Inventory; Sets Milestones; 	
void get_Boom(void){
	Milestones[0] = 0;
	Milestones[1] = 0;
	Inventory[0]--;
}

//-------------------------get_Boost()-------------------------------------
	// Remarks: Updates the Inventory; 	
void get_Boost(void){
	if(Inventory[1] > 0){ 
		Inventory[1]--;
		Flag_Fast++;
	}
}

	//-----------------------get_Caffeine()----------------------------------
	// Remarks: Updates the Inventory; 	
void get_Caffeiene(void){
	if(Inventory[1] < 3) Inventory[1]++;
	}

	//-----------------------get_Confused()----------------------------------
	// Remarks: Updates the Milestones
void get_Confused(void){
	Milestones[0] = 0;
	Flag_Lag++; 
}
	
	//-----------------------get_Drunk()-------------------------------------
	// Remarks: Updates the Milestones; Inverts the slidepot	
void get_Drunk(void){
	Milestones[0] = 0;
	Milestones[2] = 1;
	Flag_Drunk = ~Flag_Drunk;
}

//-------------------------Game Progress-----------------------------------
	// Progress Variables
	int32_t Game_Round, Game_Durration;
	int32_t	Speed_Base, Speed_Lag, Speed_Level, Speed_Boost;
	int16_t	Object_Index; // Range = {0, 49}
	// Position Variables
		// Position_Name[i] = {Last, Next)
	int8_t Invert_Enemy;
	uint16_t Position_Enemy[2], Position_Player[2];
	uint16_t Boundary_Left_Enemy, Boundary_Right_Enemy;
	uint16_t Boundary_Left_Player, Boundary_Right_Player;

	//------------------------Position_Functions
void Go_Left_Enemy(int16_t i){
	if(i > Boundary_Left_Enemy){
		Position_Enemy[1] -=2;
	}
	else{
		Invert_Enemy = 0;
		Position_Enemy[1] +=2;
	}
}
void Go_Right_Enemy(int16_t i){
	if(i < Boundary_Right_Enemy){
		Position_Enemy[1] +=2;
	}
	else{
		Invert_Enemy = 1;
		Position_Enemy[1] -=2;
	}
}


void Direct_Enemy(int8_t i){

	Position_Enemy[0] = Position_Enemy[1];
	if(i == 1) Go_Left_Enemy(Position_Enemy[0]);
	else Go_Right_Enemy(Position_Enemy[0]);
}

void Direct_Player(int8_t i){
	Position_Player[0] = Position_Player[1];
	if(i == 0){
		Position_Player[1] = (Boundary_Right_Player*Slide_Value)/(4095);
	}
	else{
		Position_Player[1] = Boundary_Right_Player - (Boundary_Right_Player *(Slide_Value)/(4095));
	}
}

void Draw_Objects(int16_t random, Prop drop){
	
//	if(New_Object(random == 1)){
//		Objects[Object_Index].toggle = 1;
//		Objects[Object_Index].object = drop;
//		Objects[Object_Index].position_x = Position_Enemy[1]-2;
//		Objects[Object_Index].position_y = 9;
//		Object_Index++;
//	}
	
	
	// There is objects_n need to be linked!
	for(int i = 0; i < 50; i++){
			if ((Objects[i].toggle == 1)&&(Objects[i].object.number == 1)){
			Objects[i].position_y +=2;
			ST7735_DrawBitmap(Objects[i].position_x, Objects[i].position_y,
			object_1, Objects[i].object.width,Objects[i].object.height);
			}
			if ((Objects[i].toggle == 1)&&(Objects[i].object.number == 2)){
			Objects[i].position_y +=2;
			ST7735_DrawBitmap(Objects[i].position_x, Objects[i].position_y,
			object_2, Objects[i].object.width,Objects[i].object.height);
			}
			if ((Objects[i].toggle == 1)&&(Objects[i].object.number == 3)){
			Objects[i].position_y +=2;
			ST7735_DrawBitmap(Objects[i].position_x, Objects[i].position_y,
			object_3, Objects[i].object.width,Objects[i].object.height);
			}
			if ((Objects[i].toggle == 1)&&(Objects[i].object.number == 4)){
			Objects[i].position_y +=2;
			ST7735_DrawBitmap(Objects[i].position_x, Objects[i].position_y,
			object_1, Objects[i].object.width,Objects[i].object.height);
			}
		}
}
//--------------------Timers and Countdowns---------------------------
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}


//-----------------------Get_Started()-----------------------------------
	// Remarks: 
	// 				  Updates the Milestones and Inventory to starting levels.
	// 				  Updates the Game_Variables to starting levels.
	// 				  OBJECTS NEED TO BE CLEARED BEFORE INITIALIZING!
	// 				  Updates the Milestones and Inventory to starting levels.
	// 				  Updates the Milestones and Inventory to starting levels.
void Get_Started(void){
	Flag_Drunk = 0;
	Flag_Lag = 0;
	Flag_Fast = 0;
	Milestones[0] = 1;
	Milestones[1] = 1;
	Milestones[2] = 0;
	Inventory[0] = 3;
	Inventory[1] = 0;
	Speed_Base = 1;
	Speed_Lag = 0;
	Speed_Level = 0;
	Speed_Boost = 0;
	Game_Round = 0;
	Object_Index = 0;
}

//---------------------------TESTS--------------------------------------
// Remarks: In some cases, these test functions build on the previous 
//					ones because they are more complex. Use the minimal level
//					of complexity required. In other cases, these functions are
//					protypes. Get_Creative().  

	//-----------------------Test_Init()
	// Remarks:	Test_Init is used for setting functions and variables that 
	// 					are required for game play, but are undecided or ideal
	//				 	constants that will used in the overall game.
void Test_Init(void){
	Position_Enemy[0] = 0;
	Position_Enemy[1] = 0;
	Position_Player[0] = 0;
	Position_Player[1] = 0;
	Invert_Enemy = 0;

	Game_Durration = 600; // 15 sec
	Boundary_Left_Enemy = 0, Boundary_Right_Enemy = 110;
	Boundary_Left_Player = 0, Boundary_Right_Player = 110;
	
	Objects[0].toggle = 1;
	Objects[0].object.number = 1;
	Objects[0].position_x = 0;
	Objects[0].position_y = 9;
	Objects[0].object.height = 29;
	Objects[0].object.width = 15;

	Objects[1].toggle = 1;
	Objects[1].object = TNT;
	Objects[1].position_x = 55;
	Objects[1].position_y = 15;
}
void Test_Image(void){
	Output_Init(); 
  ST7735_FillScreen(0x0000);					        			    	// set screen to black
  ST7735_DrawBitmap(0,159, Start_Screen, 128, 160);
  ST7735_DrawBitmap(0, 159, PlayerShip0, 18,8);				 // player ship middle bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);

  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);


  Delay100ms(10);					              									// delay 1 sec at 80 MHz


  ST7735_FillScreen(0x0000);      									      // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  LCD_OutDec(1234);
	}

void Test_Inputs(void){
	// In progress....
}

int16_t Test_Formula_Speed(void){
	return (Speed_Base + Speed_Level + Speed_Lag - Speed_Boost);
}
void Test_Movement_Enemy(void){
		ST7735_FillScreen(0x0000);
		Direct_Enemy(Invert_Enemy);

		ST7735_FillScreen(0x0000);	
		ST7735_DrawBitmap(Position_Enemy[1], 9, SmallEnemy10pointA, 16,10);
	  ST7735_DrawBitmap(Position_Player[1], 159, PlayerShip0, 18,8);
		Delay100ms(Test_Formula_Speed());
}

int main(void){

  TExaS_Init();  																				// Set system clock to 80 MHz
	Random_Init(1);
	PortF_Init();
	ADC_Init(); 																					// Switch = PE0; Slide = PE2;
	SysTick_Init();
	Output_Init();

	// *The following code needs to be tested for bugs*
	Test_Init();
	Get_Started();
	EnableInterrupts();

	
  while(1){
			while(Game_Durration != 0){
				Test_Movement_Enemy();
				Draw_Objects(1, Coffee);
			}
  }

}

	//------------------------SysTick_Handler---------------------------
	// Input: 		None
	// Output:		None
	// Operation: Returns the player I/O;

void SysTick_Handler(void){
    Heartbeat_Blue ^= 0x04;      	// Profile ADC
    Heartbeat_Blue ^= 0x04;      	// Profile ADC
	
    Slide_Value = ADC_In();  			// Sample 12-bit channel 1
		Select_Value = PE0;
		ADCStatus = 1;
	if(Game_Durration > 0) {Game_Durration--;}
	Heartbeat_Blue ^= 0x04;       // Profile ADC
	}
