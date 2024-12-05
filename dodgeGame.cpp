#define _CRT_SECURE_NO_WARNINGS
#include <glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include<time.h>
#include <cmath>

#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 700
#define TIMER_PERIOD  20
#define TIMER_ON 1
#define D2R 0.0174532
#define PI 3.1415
#define MAX_FIRE 15
#define MAX_FIRERATE 9
#define MIN_FIRERATE 1
#define MAX_PROJSPEED 15
#define MIN_PROJSPEED 5
#define SPEED 5

//defining states of the machine
#define MENU 0
#define OPTIONS 1
#define RUN 2
#define GAMEOVER 3
#define USER 4
#define HIGHSCORE 5


//defining point data structure
typedef struct {
	float x, y;
} point_t;


//defining fire type(the random objects shooting towards the player), using nested structures
typedef struct {
	point_t startPos;
	point_t endPos;
	point_t currentPos;
	float angle;
	bool active;
} fire_t;

typedef struct {
	char uname[40];
	double score;
}player_t;

//Global variables declaration
//declaration of X and Y values of the player object
double currentX = 0, currentY = 0, startX = 0, startY = 0, destX, destY;

double speed = SPEED,		//default speed of player obj
xSpeed, ySpeed;				//variables for seprated X and Y speed vectors

//String needed for user inputs, and displaying outputs
char fRateString[10], pSpeedString[10], scoreString[20], username[40]="";


int clockFlag = 0,		
chance1, chance2,		//used for frequency control
hp = 3,					//default hitpoints
state = MENU,			//default state
fireRate = 5,			//default fireRate
fRateCircleX = 0,		//X coordinate of the circle in the range menu, showing the value of fireRate
projSpeed = 10, 
pSpeedCircleX = 0;		//X coordinate of the circle in the range menu, showing the value of projectileSpeed

double score = 0, multiplier = 2;

fire_t fire[MAX_FIRE];			//an array to store all the fires(data types), limiting the number to MAX_FIRE
player_t arr[40];				//array for storing player names and scores temporarily


void resetVar()
{
	currentX = 0, currentY = 0, startX = 0, startY = 0, destX = 0, destY = 0;
	hp = 3;
	score = 0;
	*username = '\0';
	for (int i = 0; i < MAX_FIRE; i++)
	{
		fire[i].active = false;
		*(username + i) = '\0';
	}
}

void writeScore()
{
	FILE* inp = fopen("scores.txt", "a");
	fprintf(inp, "\n%.2f %s", score, username);
	fclose(inp);
}

void swap(player_t* a, player_t* b)
{
	player_t temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

void desBubbleSort(player_t* arr, int size)
{
	if (size != 1)
	{
		for (int i = 0; i < size - 1; i++)
			if (arr[i].score < arr[i + 1].score)
				swap(&arr[i], &arr[i + 1]);

		desBubbleSort(arr, size - 1);
	}
}

void vprint(int x, int y, void* font, const char* string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, str[i]);
}

void vprint2(int x, int y, float size, const char* string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	glPopMatrix();
}


//implementing the UI for Options menu; calculating the X position for the circle in the fire rate range setting 
void calc_fRateCircleX()
{
	
	fRateCircleX = 600 * (fireRate - 1) / 8 - 300;
}


//implementing the UI for Options menu; calculating the X position for the circle in the speed range setting
void calc_pSpeedCircleX()
{
	
	pSpeedCircleX = 600 * (projSpeed - 5) / 10 - 300;
}


//calculating the score multiplier according to the difficulty(fire_rate and projectile speed)
void calcMultiplier()
{
	multiplier = fireRate * 5.0 / MAX_FIRERATE + (projSpeed - 4) * 5.0 / (MAX_PROJSPEED - 4);
}



//fire count limitation mechanism, using an array
//A mechanism to find any fires that are not active already, and setting them to active, so they will start moving
int findAvailableFire() {
	for (int i = 0; i < MAX_FIRE; i++)
		if (fire[i].active == false) return i;
	return -1;
}


//functions to draw primitive shapes
void circle(int x, int y, int r) {
	float angle;

	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++) {
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}


void drawHeart(float centerX, float centerY) {
	glBegin(GL_LINE_LOOP);

	for (float t = 0; t <= 2 * PI; t += 0.01) {
		float x = centerX + 16 * pow(sin(t), 3);
		float y = centerY + 13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t);

		glVertex2f(x, y);
	}

	glEnd();
}

void drawFilledHeart(float centerX, float centerY) {
	glBegin(GL_POLYGON);

	for (float t = 0; t <= 2 * PI; t += 0.01) {
		float x = centerX + 16 * pow(sin(t), 3);
		float y = centerY + 13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t);

		glVertex2f(x, y);
	}

	glEnd();
}



//iterating through the fires array, and rendering the ones with active status according to their X and Y.
void drawFires() {
	for (int i = 0; i < MAX_FIRE; i++) {
		if (fire[i].active) {
			glColor3f(1, 1, 0);
			circle(fire[i].currentPos.x, fire[i].currentPos.y, 20);
		}
	}
}


//Given any fire type, tests the collision with the player object
bool testCollision(fire_t fr, int currentX, int currentY) {
	float dx = currentX - fr.currentPos.x;
	float dy = currentY - fr.currentPos.y;
	float d = sqrt(dx * dx + dy * dy);
	return d <= 50;
}





void displayMenu()
{
	resetVar();

	glColor3f(1, 1, 1);
	glRectf(-450, -250, 450, 250);

	glColor3ub(1, 102, 255);
	vprint2(-200, 150, 0.45, "Dodge Game");

	glColor3ub(0, 51, 128);
	glRectf(-300, 90, 300, 40);
	glRectf(-300, 10, 300, -40);
	glRectf(-300, -70, 300, -120);

	glColor3f(1, 1, 1);
	vprint2(-180, 55, 0.27, "Play");
	vprint2(-180, -25, 0.27, "Options");
	vprint2(-180, -105, 0.27, "High Scores");

}

void onKeyboard(unsigned char key, int x, int y)
{
	if (state == USER) {		//appending one character at a time to the string.
		if (((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z')) && strlen(username) < 30) {
			username[strlen(username)] = key;
			username[strlen(username) + 1] = '\0';
		}

		else if (key == '\b' && strlen(username) > 0)		//deleting from the charachter using backspace
			username[strlen(username) - 1] = '\0';
		else if (key == 13)		//entering the data using the ENTER button
			state = RUN;
	}
}	



void displayUser()		//display function for USER state
{
	glColor3f(1, 1, 1);
	glRectf(-450, -250, 450, 250);

	glColor3ub(0, 75, 168);
	glRectf(50, 20, 270, -20);

	glColor3f(0, 0, 0);
	vprint2(-250, -9, 0.2, "Enter your username:  ");
	glColor3f(1, 1, 1);
	glLineWidth(0.8);
	vprint2(57, -9, 0.18, username);

}

void displayOptions()		//displaying options menu and all the UI elements like range bar
{
	glColor3f(1, 1, 1);
	glRectf(-450, -250, 450, 250);

	glColor3ub(1, 102, 255);
	vprint2(-100, 150, 0.45, "Options");

	glColor3ub(0, 51, 128);
	glRectf(-420, 230, -350, 190);
	glColor3f(1, 1, 1);
	glRectf(-396, 215, -363, 205);

	glBegin(GL_TRIANGLES);
	glVertex2f(-396, 220);
	glVertex2f(-396, 200);
	glVertex2f(-406, 210);
	glEnd();
	
	glColor3ub(0, 71, 179);
	glRectf(-300, 30, 300,26);
	vprint2(-70, 55, 0.2, "Firing Rate");
	glRectf(-300, -120, 300, -124);
	vprint2(-90, -95, 0.2, "Prijectile Speed");

	glColor3f(0, 0, 0);
	circle(fRateCircleX, 28, 7);
	circle(pSpeedCircleX, -122, 7);

	glColor3ub(0, 51, 128);
	glRectf(40, 18, -40, -12);
	glRectf(-50, 12, -74, -9);
	glRectf(50, 12, 74, -9);

	glRectf(40, -132, -40, -162);
	glRectf(-50, -138, -74, -159);
	glRectf(50, -138, 74, -159);

	sprintf(fRateString, "%d", fireRate);
	sprintf(pSpeedString, "%d", projSpeed);
	glColor3f(1, 1, 1);
	vprint2(-5, -5, 0.15, fRateString);
	vprint2(-67, -4, 0.1, "-");
	vprint2(58, -4, 0.1, "+");

	vprint2(-12, -155, 0.15, pSpeedString);
	vprint2(-67, -154, 0.1, "-");
	vprint2(58, -154, 0.1, "+");



}

void displayRun()
{
	glColor3f(0, 0, 0);
	vprint2(-680, 330, 0.15, scoreString);

	glColor3f(1, 0, 0);
	circle(currentX, currentY, 30);

	drawFires();

	glColor3f(1, 0, 0);
	drawHeart(610, -300);
	drawHeart(570, -300);
	drawHeart(530, -300);

	

	switch (hp)		//drawing the hearts representing your extra lives in the game
	{
	case 3:	drawFilledHeart(610, -300);
		drawFilledHeart(570, -300);
		drawFilledHeart(530, -300);
		break;
	case 2:	drawFilledHeart(610, -300);
		drawFilledHeart(570, -300);
		break;
	case 1: drawFilledHeart(610, -300);
		break;

	default:
		writeScore();		//write the score to the file
		state = GAMEOVER;	//go to GAMEOVER state

	}
}

void displayGameOver()
{
	glColor3f(1, 1, 1);
	glRectf(-450, -250, 450, 250);

	sprintf(scoreString, "%.0f", score);
	glColor3f(0, 0, 0);
	vprint2(90, 0, 0.32, scoreString);
	vprint2(-200, 0, 0.30, "You Scored:  ");

	glColor3ub(0, 51, 128);
	glRectf(-420, 230, -350, 190);
	glColor3f(1, 1, 1);
	glRectf(-396, 215, -363, 205);

	glBegin(GL_TRIANGLES);
	glVertex2f(-396, 220);
	glVertex2f(-396, 200);
	glVertex2f(-406, 210);
	glEnd();

}

void displayHighScore()
{
	char tempStr[40];

	FILE* inp = fopen("scores.txt", "r");		//file to read the score from
	int size = 0;
	
	while (fscanf(inp, "%lf %[^\n]", &arr[size].score, arr[size].uname) != EOF) // read opertaion
		size++;
	
	fclose(inp);

	desBubbleSort(arr, size);  //descending bubble sort



	//UI elements
	glColor3f(1, 1, 1);
	glRectf(-450, -250, 450, 250);
	glColor3ub(0, 51, 128);
	glRectf(-250, 150, 250, -180);
	
	glRectf(-420, 230, -350, 190);
	glColor3f(1, 1, 1);
	glRectf(-396, 215, -363, 205);

	glBegin(GL_TRIANGLES);
	glVertex2f(-396, 220);
	glVertex2f(-396, 200);
	glVertex2f(-406, 210);
	glEnd();

	glColor3f(0, 0, 0);
	glLineWidth(5);
	glBegin(GL_LINES);
	glVertex2f(-250, 60);
	glVertex2f(250, 60);
	glVertex2f(-250, -20);
	glVertex2f(250, -20);
	glVertex2f(-250, -100);
	glVertex2f(250, -100);
	glEnd();

	glLineWidth(3);
	glColor3f(1, 1, 1);
	vprint2(-112, 100, 0.3, "High Scores");
	vprint2(-200, 20, 0.2, "1.");
	vprint2(-200, -60, 0.2, "2.");
	vprint2(-200, -140, 0.2, "3.");

	int d = 0;

	for (int i = 0; i < 3; i++)
	{
		sprintf(tempStr, "%s", arr[i].uname);
		vprint2(-150, 20-d, 0.2, tempStr);
		sprintf(tempStr, "%.0f", arr[i].score);
		vprint2(100, 20-d, 0.2, tempStr);

		d += 80;
	}

}

void display() {
	// Draw window
	glClearColor(0.8, 0.8, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glLineWidth(3);

	switch (state)		//call the display function of each state
	{
	case MENU: displayMenu(); 
		break;
	case RUN: displayRun(); 
		calcMultiplier();
		break;
	case OPTIONS: displayOptions();
		break;
	case GAMEOVER: displayGameOver();
		break;
	case USER: displayUser();
		break;
	case HIGHSCORE: displayHighScore();
		break;
	}

	

	glutSwapBuffers();
}

void onResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Init() {
	//Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

}

void onClick(int button, int stat, int x, int y) {
	if (state == MENU) {		//menu state on click events designated to the areas with the buttons

		if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN) {
			
			if ((x - WINDOW_WIDTH / 2.0) > -300 && (x - WINDOW_WIDTH / 2.0) < 300 && -(y - WINDOW_HEIGHT / 2.0) > 40 && -(y - WINDOW_HEIGHT / 2.0) < 90)
				state = USER;

			if ((x - WINDOW_WIDTH / 2.0) > -300 && (x - WINDOW_WIDTH / 2.0) < 300 && -(y - WINDOW_HEIGHT / 2.0) > -40 && -(y - WINDOW_HEIGHT / 2.0) < 10)
				state = OPTIONS;

			if ((x - WINDOW_WIDTH / 2.0) > -300 && (x - WINDOW_WIDTH / 2.0) < 300 && -(y - WINDOW_HEIGHT / 2.0) > -120 && -(y - WINDOW_HEIGHT / 2.0) < -70)
				state = HIGHSCORE;
		}

		
	}

	else if (state == OPTIONS)		//usage of the range bars in the options menu
	{
		
		if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN) {
			if ((x - WINDOW_WIDTH / 2.0) > -420 && (x - WINDOW_WIDTH / 2.0) < -350 && -(y - WINDOW_HEIGHT / 2.0) > 190 && -(y - WINDOW_HEIGHT / 2.0) < 230)
				state = MENU;			//back button to menu

			else if ((x - WINDOW_WIDTH / 2.0) > -74 && (x - WINDOW_WIDTH / 2.0) < -50 && -(y - WINDOW_HEIGHT / 2.0) > -9 && -(y - WINDOW_HEIGHT / 2.0) < 12)
			{
					if (fireRate > MIN_FIRERATE) {	//decrease fireRate button is pressed

						fireRate--;
						calc_fRateCircleX();
					}
			}
			else if ((x - WINDOW_WIDTH / 2.0) > 50 && (x - WINDOW_WIDTH / 2.0) < 74 && -(y - WINDOW_HEIGHT / 2.0) > -9 && -(y - WINDOW_HEIGHT / 2.0) < 12)
			{
					if (fireRate < MAX_FIRERATE) {	//increase fireRate button is pressed

						fireRate++;
						calc_fRateCircleX();

					}
			}

			else if ((x - WINDOW_WIDTH / 2.0) >50 && (x - WINDOW_WIDTH / 2.0) < 74 && -(y - WINDOW_HEIGHT / 2.0) > -159 && -(y - WINDOW_HEIGHT / 2.0) < 162)
			{
				if (projSpeed < MAX_PROJSPEED) {	//increase project speed button is pressed

					projSpeed++;
					calc_pSpeedCircleX();

				}
			}

			else if ((x - WINDOW_WIDTH / 2.0) > -74 && (x - WINDOW_WIDTH / 2.0) < -50 && -(y - WINDOW_HEIGHT / 2.0) > -159 && -(y - WINDOW_HEIGHT / 2.0) < 162)
			{
				if (projSpeed > MIN_PROJSPEED) {		//decrease project speed button is pressed

					projSpeed--;
					calc_pSpeedCircleX();
				}
			}

		}

			
		
		
	}

	else if (state == RUN && button == GLUT_RIGHT_BUTTON && stat == GLUT_DOWN) { //command to move the object
		destX = (x - WINDOW_WIDTH / 2.0);		//converting screen coordinates to OpenGL coordinates
		destY = -(y - WINDOW_HEIGHT / 2.0);
		startX = currentX;		//set the start point of the animation
		startY = currentY;

		//split the velocity vector to xSpeed and ySpeed, and calculate their values.
		xSpeed = speed * (destX - startX) / sqrtf(pow(destX - startX, 2) + pow(destY - startY, 2));
		ySpeed = speed * (destY - startY) / sqrtf(pow(destX - startX, 2) + pow(destY - startY, 2));

	}

	else if ((state == HIGHSCORE || state == GAMEOVER)  && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN)
		
		if ((x - WINDOW_WIDTH / 2.0) > -440 && (x - WINDOW_WIDTH / 2.0) < -350 && -(y - WINDOW_HEIGHT / 2.0) > 190 && -(y - WINDOW_HEIGHT / 2.0) < 230)
				state = MENU;

	
	glutPostRedisplay();
}

void onTimer(int v)
{
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	

	
	if (state == RUN)
	{
		clockFlag++;		//an abstract variable to control the frequency the different events


		if (clockFlag % (100/fireRate) == 0)
		{

			int availFire = findAvailableFire();		//find available fires 
			
			if (availFire != -1)
			{
				fire[availFire].active = true;			//set available fire to active(start it)
				chance2 = rand() % 2;					//random end position determining
				if (chance2) {	
					fire[availFire].startPos.x = rand() % 1400 - 700;	//TOP-BOTTOM MOVEMENT
					fire[availFire].endPos.x = rand() % 1400 - 700;		//random x values 
					chance1 = rand() % 2;
					if (chance1)
					{					//top to bootm,
						fire[availFire].startPos.y = 350;	
						fire[availFire].endPos.y = -350;
					}
					else
					{					//or bottom to top
						fire[availFire].startPos.y = -350;
						fire[availFire].endPos.y = 350;
					}
				}

				else
				{
					fire[availFire].startPos.y = rand() % 700 - 350;	//RIGHT-LEFT MOVEMENT
					fire[availFire].endPos.y = rand() % 700 - 350;		//random y values
					chance1 = rand() % 2;
					if (chance1)
					{					//right to loft
						fire[availFire].startPos.x = 700;
						fire[availFire].endPos.x = -700;
					}
					else
					{					//or left to right
						fire[availFire].startPos.x = -700;
						fire[availFire].endPos.x = 700;
					}
				}

				fire[availFire].currentPos = fire[availFire].startPos; // start the animation from start pos
				

			}
		}

		if (clockFlag % 2 == 0)
		{
			score += multiplier;	 //incrementing and displaying the score according to the calculated multiplier
			printf("%.2f,  %.2f,   %s\n", score, multiplier, username);
			sprintf(scoreString, "%09.0f", score);
		}

		for (int i = 0; i < MAX_FIRE; i++) {
			if (fire[i].active) {		//moving the fire object
				fire[i].currentPos.x += projSpeed * (fire[i].endPos.x - fire[i].startPos.x) / sqrtf(pow(fire[i].endPos.x - fire[i].startPos.x, 2) + pow(fire[i].endPos.y - fire[i].startPos.y, 2));

				fire[i].currentPos.y += projSpeed * (fire[i].endPos.y - fire[i].startPos.y) / sqrtf(pow(fire[i].endPos.x - fire[i].startPos.x, 2) + pow(fire[i].endPos.y - fire[i].startPos.y, 2));


				//terminating the fire object once it exits the window
				if (fire[i].currentPos.x > WINDOW_WIDTH / 2 || fire[i].currentPos.x < -WINDOW_WIDTH / 2 || fire[i].currentPos.y > WINDOW_HEIGHT / 2 || fire[i].currentPos.y < -WINDOW_HEIGHT / 2)
					fire[i].active = false;
				if (testCollision(fire[i], currentX, currentY)) {
					fire[i].active = false;
					hp--;
				}
			}
		}



		//to stop the player object from moving
		if (currentX - destX<1 && currentX - destX>-1 && currentY - destY<1 && currentY - destY>-1) {
			startX = destX;
			startY = destY;

		}

		//animating the player object according to the preset speeds
		else {
			currentX += xSpeed;
			currentY += ySpeed;
		}
	}
	

	

	// To refresh the window it calls display() function
	glutPostRedisplay(); // display()
}


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("DodgeGame by Moein Faghih");
	srand(time(NULL));


	// Window Events
	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);

	glutMouseFunc(onClick);

	glutKeyboardFunc(onKeyboard);

	Init();
	glutMainLoop();
	return 0;
}