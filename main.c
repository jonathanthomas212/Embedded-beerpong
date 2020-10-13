#include <stdio.h>
#include <lpc17xx.h>
#include <cmsis_os2.h>
#include "GLCD.h"
#include "ball.c"
#include "power.c"
#include <stdbool.h>

//define struct with p1 and p2 values
struct player{
	bool cupStatus[6];
	int cupPosition[3];
	int x_init, y_init;
	char *turn, *win;
};
struct player p1, p2;

int turnCount = 0;
int x_curr, y_curr, shotPower, cupNum, fillCount;
double shotPowerDouble;

//set all strings
char *click1 = "Click pushbutton";
char *click2 = "to Play Beer Pong!";
char *click3 = "Click pushbutton";
char *click4 = "when done!";
char *instruction1 = "INSTRUCTIONS:";
char *instruction2 = "1.  Set the power";
char *instruction3 = "using potentiometer";
char *instruction4 = "2.   Adjust your";
char *instruction5 = "  position with";
char *instruction6 = " joystick";
char *instruction7 = "3.Use pushbutton to";
char *instruction8 = "SHOOOOOOOOT";
char *calibrate1 = "Twist the";
char *calibrate2 = "potentiometer fully";
char *calibrate3 = "to calibrate it.";
char *bangBang = "BANGBANG";
char *miss = "Missed:(";

void nextTurn() {
	//set ball position for each turn and increment turnCount
	if (turnCount % 2 == 0) {
		x_curr = p2.x_init;
		y_curr = p2.y_init;
	} else if (turnCount %2 != 0) {
		x_curr = p1.x_init;
		y_curr = p1.y_init;
	}
	GLCD_Clear(DarkGreen);
	turnCount++;
}

bool checkWinner(bool arr[]) {
	for (int i = 0; i < 6; i++) {
		if (!arr[i])
			return false;
	}
	return true;
}

void checkStatus(bool cupStatus[], char *winner) {
	//if cup has not been hit yet:
	if (cupStatus[cupNum] == false && cupNum != -1) {
		cupStatus[cupNum] = true;
		if (checkWinner(cupStatus)) {
			GLCD_DisplayString(1, 6, 1, (unsigned char *) bangBang);
			osDelay(2000);
			GLCD_Clear(DarkGreen);
			GLCD_DisplayString(3, 6, 1, (unsigned char *) winner);
			osDelay(osWaitForever);
		} else {
			GLCD_DisplayString(1, 6, 1, (unsigned char *) bangBang);
			osDelay(2000);
		}
		} else { // if already hit, no more cup in that position:
		GLCD_DisplayString(1, 6, 1, (unsigned char *) miss);
		osDelay(2000);
	}
}

void updateCupPosition(bool cupStatus[], int cupPosition[], char *text) {
	GLCD_Clear(DarkGreen);
	GLCD_DisplayString(1, 6, 1, (unsigned char *) text);
	
	//checks if cups have been hit and if not, set cups on screen
	if (!cupStatus[0])
		GLCD_Bitmap(cupPosition[0], 110, 20, 20, (unsigned char *) cup20);
	if (!cupStatus[1])
		GLCD_Bitmap(cupPosition[1], 99, 20, 20, (unsigned char *) cup20);
	if (!cupStatus[2])
		GLCD_Bitmap(cupPosition[1], 121, 20, 20, (unsigned char *) cup20);
	if (!cupStatus[3])
		GLCD_Bitmap(cupPosition[2], 88, 20, 20, (unsigned char *) cup20);
	if (!cupStatus[4])
		GLCD_Bitmap(cupPosition[2], 110, 20, 20, (unsigned char *) cup20);
	if (!cupStatus[5])
		GLCD_Bitmap(cupPosition[2], 132, 20, 20, (unsigned char *) cup20);
}

void updateCupMemory(bool cupStatus[], int cupPosition[], char *winner) {
	cupNum = -1;
	
	//if it hits the position of a cup, doesnt matter if cup was already hit
	if (x_curr >= (cupPosition[0] + 1) && x_curr <= (cupPosition[0] + 9) && y_curr >= 111 && y_curr <= 119) {
		cupNum = 0;
		checkStatus(cupStatus, winner);
	} else if (x_curr >= (cupPosition[1] + 1) && x_curr <= (cupPosition[1] + 9) && y_curr >= 100 && y_curr <= 108) {
		cupNum = 1;
		checkStatus(cupStatus, winner);
	} else if (x_curr >= (cupPosition[1] + 1) && x_curr <= (cupPosition[1] + 9) && y_curr >= 122 && y_curr <= 130) {
		cupNum = 2;
		checkStatus(cupStatus, winner);
	} else if (x_curr >= (cupPosition[2] + 1) && x_curr <= (cupPosition[2] + 9) && y_curr >= 89 && y_curr <= 97) {
		cupNum = 3;
		checkStatus(cupStatus, winner);
	} else if (x_curr >= (cupPosition[2] + 1) && x_curr <= (cupPosition[2] + 9) && y_curr >= 111 && y_curr <= 119) {
		cupNum = 4;
		checkStatus(cupStatus, winner);
	} else if (x_curr >= (cupPosition[2] + 1) && x_curr <= (cupPosition[2] + 9) && y_curr >= 133 && y_curr <= 141) {
		cupNum = 5;
		checkStatus(cupStatus, winner);
	}
	
	//if the shot misses
	if (cupNum == -1) {
		GLCD_DisplayString(1, 6, 1, (unsigned char *) miss);
		osDelay(2000);
	}
}

void setPowerFill() {
	if (fillCount <= 45 && fillCount >= 0) {
		for (int i = 0; i < 45; i++) {
			if (i <= fillCount) //fills up orange part of the power bar
				GLCD_Bitmap(138 + i, 203, 1, 9, (unsigned char *) powerBoxFill);
			else // fills up remainder of the power bar
				GLCD_Bitmap(138 + i, 203, 1, 9, (unsigned char *) powerBoxBackfill);
		}
	}
	//shooting math to get correct pixel to offset by on screen
	shotPowerDouble = fillCount * (150 / 40);
	shotPower = shotPowerDouble;
	shotPower += 165;
}

void joystick(void *arg) {
	while ((LPC_GPIO2 -> FIOPIN & (1 << 10))) {}; //this is used to prevent the joystick from showing up on the potentiometer calibration page, 
																								//because this thread is already declared and would otherwise give the user the ability to
																								//move the joystick from its default position before it's even displayed
	while (1) {			
		if (!(LPC_GPIO1->FIOPIN & (1<<23)) && y_curr >= 20) { //joystick moves up
			if (y_curr != 20)
				y_curr -= 1;
		}
		if (!(LPC_GPIO1->FIOPIN & (1<<25)) && y_curr <= 200) { //joystick moves down
			if (y_curr != 200)	
				y_curr += 1;
		}
		osDelay(15);
		osThreadYield();
	}
}

void potentiometer(void *arg) {
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (1 << 12);
	LPC_ADC->ADCR = (1 << 2) | // select AD0.2 pin
									(4 << 8) | // ADC clock is 25MHz/5
									(0 << 24)| // Do not start conversion yet 
									(1 << 21); // enable ADC
	int power_arr[3]; 
	int count = 0; 
	int dif = 0;
	int difMin = 1;
	double frameVal;

	while(1) {
		LPC_ADC -> ADCR |= (1 << 24);
		while(!(LPC_ADC -> ADGDR & (1U << 31))) {}
		// continuously updates current and previous values from the potentiometer
			if(count == 5){
			int prev = power_arr[2]; 
			*power_arr = NULL; 
			power_arr[0] = prev;  
			count = 1; 
		}
		
		power_arr[count] = (LPC_ADC -> ADGDR >> 4) & 0xFFF;
		dif = power_arr[count] - power_arr[count-1]; // computes the difference to give us a usable value for power
		if (dif < difMin && dif > -15000) //sets a minimum with a range upwards of 4096
			difMin = dif;
		frameVal = ((dif - difMin) / 4096.0);
		frameVal *= 45;
		fillCount = frameVal;

		osThreadYield();
	}
}

void updateMonitor(void *arg) {
	while ((LPC_GPIO2 -> FIOPIN & (1 << 10))) {//this is used to prevent all the graphics from showing up on the potentiometer calibration page, 
																						 //because this thread is already declared and would otherwise display all the graphics
		
		setPowerFill(); // shows potentiometer on calibration page before all other graphics are displayed
	};
		
	GLCD_Clear(DarkGreen);
	GLCD_DisplayString(1, 6, 1, (unsigned char *) p1.turn);

	//SET P2 CUPS TO BEGIN GAME
	GLCD_Bitmap(p2.cupPosition[0], 110, 20, 20, (unsigned char *) cup20);
	GLCD_Bitmap(p2.cupPosition[1], 99, 20, 20, (unsigned char *) cup20);
	GLCD_Bitmap(p2.cupPosition[1], 121, 20, 20, (unsigned char *) cup20);
	GLCD_Bitmap(p2.cupPosition[2], 88, 20, 20, (unsigned char *) cup20);
	GLCD_Bitmap(p2.cupPosition[2], 110, 20, 20, (unsigned char *) cup20);
	GLCD_Bitmap(p2.cupPosition[2], 132, 20, 20, (unsigned char *) cup20);
	
	while(1) {
		setPowerFill();
		GLCD_Bitmap(x_curr, y_curr + 10, 10, 10, (unsigned char *) upArrow);
		GLCD_Bitmap(x_curr, y_curr - 10, 10, 10, (unsigned char *) downArrow);
		GLCD_Bitmap(x_curr, y_curr, 10, 10, (unsigned char *) smallBall);
		
		if (turnCount % 2 == 0) {
			// PLAYER 1 SHOOTS
			if(!(LPC_GPIO2 -> FIOPIN & (1 << 10))) {
				for(int i = 0; i < 240; i++)
					GLCD_Bitmap(x_curr, i, 10, 1, (unsigned char *) greenBack);
				x_curr = shotPower;
				GLCD_Bitmap(x_curr, y_curr, 10, 10, (unsigned char *) smallBall); //repositions ball after shot
				
				updateCupMemory(p2.cupStatus, p2.cupPosition, p1.win); //updates memory after shot occurs
				nextTurn(); //sets up variables to start next turn
				updateCupPosition(p1.cupStatus, p1.cupPosition, p2.turn); //accesses memory and updates new positions
			}
		} else {
			//PLAYER 2 SHOOTS
			if(!(LPC_GPIO2 -> FIOPIN & (1 << 10))) {
				for(int i = 0; i < 240; i++)
					GLCD_Bitmap(x_curr, i, 10, 1, (unsigned char *) greenBack);
				x_curr = 310 - shotPower; 
				GLCD_Bitmap(x_curr, y_curr, 10, 10, (unsigned char *) smallBall);//repositions ball after shot
				
				updateCupMemory(p1.cupStatus, p1.cupPosition, p2.win); //updates memory after shot occurs
				nextTurn(); //sets up variables to start next turn
				updateCupPosition(p2.cupStatus, p2.cupPosition, p1.turn); //accesses memory and updates new positions
			}
		}
		osThreadYield();
	}
}

int main(void) { 
	printf("YUUUUUURRRRRR\n");
	
	//setting initial values for p1 and p2 variables
	for(int i = 0; i < 6; i++) {
		if (i < 3) {
			p1.cupPosition[i] = 85 - (i * 20);
			p2.cupPosition[i] = 215 + (i * 20);
		}
		p1.cupStatus[i] = false;
		p2.cupStatus[i] = false;
	}
	p1.x_init = 5;
	p1.y_init = 115;
	p1.turn = "P1 Turn!";
	p1.win = "P1 WINS!";
	p2.x_init = 305;
	p2.y_init = 115;
	p2.turn = "P2 Turn!";
	p2.win = "P2 WINS!";
	
	x_curr = p1.x_init;
	y_curr = p1.y_init;
	
	//display starting and instructional pages
	GLCD_Init();
	GLCD_Clear(DarkGreen);
	GLCD_SetBackColor(DarkGreen);
	GLCD_SetTextColor(White);
	GLCD_DisplayString(4, 2, 1, (unsigned char *) click1);
	GLCD_DisplayString(5, 1, 1, (unsigned char *) click2);	
	
	while ((LPC_GPIO2 -> FIOPIN & (1 << 10))) {};
	GLCD_Clear(DarkGreen);
		
	GLCD_DisplayString(0, 4, 1, (unsigned char *) instruction1);
	GLCD_DisplayString(1, 0, 1, (unsigned char *) instruction2);
	GLCD_DisplayString(2, 1, 1, (unsigned char *) instruction3);
	GLCD_DisplayString(4, 0, 1, (unsigned char *) instruction4);
	GLCD_DisplayString(5, 2, 1, (unsigned char *) instruction5);
	GLCD_DisplayString(6, 5, 1, (unsigned char *) instruction6);
	GLCD_DisplayString(8, 0, 1, (unsigned char *) instruction7);
	GLCD_DisplayString(9, 5 , 1, (unsigned char *) instruction8);
		
	while ((LPC_GPIO2 -> FIOPIN & (1 << 10))) {};
	GLCD_Clear(DarkGreen);
		
	//initialize threads
	osKernelInitialize();
	osThreadNew(potentiometer, NULL, NULL);
	osThreadNew(updateMonitor, NULL, NULL);
		
	//run potentiometer calibration page
	GLCD_DisplayString(0, 5 , 1, (unsigned char *) calibrate1);
	GLCD_DisplayString(1, 0, 1, (unsigned char *) calibrate2);
	GLCD_DisplayString(2, 3, 1, (unsigned char *) calibrate3);
	GLCD_DisplayString(5, 2, 1, (unsigned char *) click3);
	GLCD_DisplayString(6, 5, 1, (unsigned char *) click4);
		
	osThreadNew(joystick, NULL, NULL);
	osKernelStart();
}