#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <ctime>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "bits/stdc++.h"
#include <unordered_set>
#include <GL/freeglut.h>

#define WINDOW_X (900)
#define WINDOW_Y (900)
#define WINDOW_NAME "Snake"
#define TEXTURE_HEIGHT (64)
#define TEXTURE_WIDTH (64)

#define RANGE_X 30
#define RANGE_Y 30
#define INIT_FPS 5;

#define UP 1
#define DOWN -1
#define LEFT 2
#define RIGHT -2
#define SNAKE_INIT_LENGTH 3
#define SNAKE_MAX_LENGTH RANGE_X * 3
#define GAME_OVER_X 860
#define GAME_OVER_Y 338

using namespace std;

struct phash{
    inline size_t operator()(const pair<int,int> & p) const{
        const auto h1 = hash<int>()(p.first);
        const auto h2 = hash<int>()(p.second);
        return h1 ^ (h2 << 1);
    }
};

void init_GL(int argc, char *argv[], const char* windowName, int windowSizeX, int windowSizeY);
void init(int textureWidth, int textureHeight, const char* inputFileNames);
void set_callback_functions();

void glut_display();
void drawSnake();
void generateFood(int &x, int &y);
void drawFood();
void glut_reshape(int width, int height);
void glut_timer(int t);
void glut_keyboard(unsigned char key, int x, int y);
void glut_navigate(int key, int, int);
void set_texture(const char* inputFileNames);

void generateGrass();
void drawGrass();

void drawHeadUp(int x, int y);
void drawHeadDown(int x, int y);
void drawHeadLeft(int x, int y);
void drawHeadRight(int x, int y);

void drawTailUp(int x, int y);
void drawTailDown(int x, int y);
void drawTailLeft(int x, int y);
void drawTailRight(int x, int y);

void drawVerticalBody(int x, int y);
void drawHorizontalBody(int x, int y);
void drawLowerRightBody(int x, int y);
void drawLowerLeftBody(int x, int y);
void drawUpperRightBody(int x, int y);
void drawUpperLeftBody(int x, int y);

unordered_set<pair<int,int>,phash> grassPosition;
int framePerSecond = INIT_FPS;
int snakeLength = SNAKE_INIT_LENGTH;
int snakePositionX[SNAKE_MAX_LENGTH] = {RANGE_X / 2 + 3, RANGE_X / 2 + 2, RANGE_X / 2 + 1};
int snakePositionY[SNAKE_MAX_LENGTH] = {RANGE_Y / 2 + 3, RANGE_Y / 2 + 3, RANGE_Y / 2 + 3};
int foodPositionX, foodPositionY;
short snakeDirection = RIGHT;
bool gameOver = false;
bool ateFood = true;
int score = 0;
GLuint g_TextureHandles[1] = {0};
bool hasGrass = false;

void drawText(string s){
    glColor3d(0, 0, 0);

    void *font = GLUT_BITMAP_TIMES_ROMAN_24;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glRasterPos2d(-12.0 * (double)s.size() / (double)GAME_OVER_X / 1.3, -0.7);
    for (int i = 0; i < s.size(); i++){
        glutBitmapCharacter(font, s[i]);
    }

    glDisable(GL_BLEND);
}

void glut_game_over(){
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.0, 0.0);
	glVertex2d(-1, 1);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(1, 1); 
	glTexCoord2d(1.0, 1.0);
	glVertex2d(1, -0.5);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(-1, -0.5); 
	glEnd();

	glBegin(GL_POLYGON);
	glColor3d(1.0, 1.0, 1.0); 
	glVertex2d(-1.0, -1.0);
	glVertex2d(1.0, -1.0);
	glVertex2d(1.0, -0.5);
	glVertex2d(-1.0, -0.5);
	glEnd();

	drawText("Your Score is " + to_string(score) + " - Press 'q' or 'Esc' to quit.");

	glutSwapBuffers();
}

void endGame(int argc, char *argv[], int windowSizeX, int windowSizeY){
	init_GL(argc, argv, "Game Over", GAME_OVER_X, GAME_OVER_Y * 1.2);
  init(GAME_OVER_X, GAME_OVER_Y, "gameOver.png");

	glutDisplayFunc(glut_game_over);
	glutKeyboardFunc(glut_keyboard);
	glutMainLoop();
}

int main(int argc, char *argv[]){
	init_GL(argc, argv, "Snake", WINDOW_X, WINDOW_Y);
	init(TEXTURE_WIDTH, TEXTURE_HEIGHT, "Snake.png");
	set_callback_functions();	
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
	endGame(argc, argv, GAME_OVER_X, GAME_OVER_Y);
	return 0;
}

void init_GL(int argc, char *argv[], const char* windowName, int windowSizeX, int windowSizeY){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(windowSizeX, windowSizeY);
	glutCreateWindow(windowName);
}
void init(int textureWidth, int textureHeight, const char* inputFileNames){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glGenTextures(1, g_TextureHandles);

	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	set_texture(inputFileNames);

	if (!hasGrass){
		generateGrass();
		hasGrass = true;
	}
}
void set_callback_functions(){
	glutDisplayFunc(glut_display);
  glutReshapeFunc(glut_reshape);
	glutTimerFunc(0, glut_timer, 0);
	glutKeyboardFunc(glut_keyboard);
	glutSpecialFunc(glut_navigate);
}

void glut_display(){
  glClear(GL_COLOR_BUFFER_BIT);
	drawGrass();
	drawSnake();
	drawFood();
	glutSwapBuffers();
	if (gameOver){
		printf("\nScore %d\n\n", score);
		glutLeaveMainLoop();
	}
}

void drawSnake(){
	glColor3f(0.0, 1.0, 0.0);
	for (int i = snakeLength - 1; i > 0; i--){
		snakePositionX[i] = snakePositionX[i - 1];
		snakePositionY[i] = snakePositionY[i - 1];
	}
	if (snakeDirection == UP) snakePositionY[0]++;
	else if (snakeDirection == DOWN) snakePositionY[0]--;
	else if (snakeDirection == LEFT) snakePositionX[0]--;
	else snakePositionX[0]++;

	if (snakePositionX[0] < 0) snakePositionX[0] = RANGE_X - 1;
	if (snakePositionX[0] >= RANGE_X) snakePositionX[0] = 0;
	if (snakePositionY[0] < 0) snakePositionY[0] = RANGE_Y - 1;
	if (snakePositionY[0] >= RANGE_Y) snakePositionY[0] = 0;

	for (int i = 0; i < snakeLength; i++){
		if (i == 0){
			if (snakeDirection == UP) drawHeadUp(snakePositionX[i], snakePositionY[i]);
			else if (snakeDirection == DOWN) drawHeadDown(snakePositionX[i], snakePositionY[i]);
			else if (snakeDirection == LEFT) drawHeadLeft(snakePositionX[i], snakePositionY[i]);
			else if (snakeDirection == RIGHT) drawHeadRight(snakePositionX[i], snakePositionY[i]);
		}else if (i == snakeLength - 1){
			if (snakePositionX[i] == snakePositionX[i - 1]){
				if (snakePositionY[i] > snakePositionY[i - 1]) drawTailDown(snakePositionX[i], snakePositionY[i]);
				else drawTailUp(snakePositionX[i], snakePositionY[i]);
			}else{
				if (snakePositionX[i] > snakePositionX[i - 1]) drawTailLeft(snakePositionX[i], snakePositionY[i]);
				else drawTailRight(snakePositionX[i], snakePositionY[i]);
			}
		}else{
			if (snakePositionX[i] == snakePositionX[i - 1] && snakePositionX[i] == snakePositionX[i + 1]) drawVerticalBody(snakePositionX[i], snakePositionY[i]);
			else if (snakePositionY[i] == snakePositionY[i + 1] && snakePositionY[i] == snakePositionY[i - 1]) drawHorizontalBody(snakePositionX[i], snakePositionY[i]);
			else if (snakePositionX[i] > snakePositionX[i + 1] && snakePositionY[i] < snakePositionY[i - 1] || snakePositionX[i] > snakePositionX[i - 1] && snakePositionY[i] < snakePositionY[i + 1]) drawLowerRightBody(snakePositionX[i], snakePositionY[i]);
			else if (snakePositionX[i] < snakePositionX[i + 1] && snakePositionY[i] < snakePositionY[i - 1] || snakePositionX[i] < snakePositionX[i - 1] && snakePositionY[i] < snakePositionY[i + 1]) drawLowerLeftBody(snakePositionX[i], snakePositionY[i]);
			else if (snakePositionX[i] > snakePositionX[i + 1] && snakePositionY[i] > snakePositionY[i - 1] || snakePositionX[i] > snakePositionX[i - 1] && snakePositionY[i] > snakePositionY[i + 1]) drawUpperRightBody(snakePositionX[i], snakePositionY[i]);
			else drawUpperLeftBody(snakePositionX[i], snakePositionY[i]);
		}
	}
	if (snakePositionX[0] == foodPositionX && snakePositionY[0] == foodPositionY){
		snakeLength++;
		score++;
		framePerSecond++;
		if (snakeLength > SNAKE_MAX_LENGTH){
			snakeLength = SNAKE_MAX_LENGTH;
		}
		ateFood = true;
	}

	if (grassPosition.find(make_pair(snakePositionX[0], snakePositionY[0])) != grassPosition.end()){
		gameOver = true;
		return;
	}

	for(int i = 1; i < snakeLength; i++){
    if (snakePositionX[i] == snakePositionX[0] && snakePositionY[i] == snakePositionY[0])
      gameOver = true;
	}
}

void drawHeadUp(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.0, 0.0);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.25, 0.0);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.25, 0.25);
	glVertex2i(x, y);
	glTexCoord2d(0.0, 0.25);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawHeadDown(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.5, 0.0);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.75, 0.0);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.75, 0.25);
	glVertex2i(x, y);
	glTexCoord2d(0.5, 0.25);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawHeadLeft(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.25, 0.0);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.5, 0.0);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.5, 0.25);
	glVertex2i(x, y);
	glTexCoord2d(0.25, 0.25);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawHeadRight(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.75, 0.0);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(1.0, 0.0);
	glVertex2i(x, y + 1); 
	glTexCoord2d(1.0, 0.25);
	glVertex2i(x, y);
	glTexCoord2d(0.75, 0.25);
	glVertex2i(x + 1, y); 
	glEnd();
}

void drawTailUp(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.0, 0.25);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.25, 0.25);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.25, 0.5);
	glVertex2i(x, y);
	glTexCoord2d(0.0, 0.5);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawTailDown(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.5, 0.25);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.75, 0.25);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.75, 0.5);
	glVertex2i(x, y);
	glTexCoord2d(0.5, 0.5);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawTailLeft(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.25, 0.25);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.5, 0.25);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.5, 0.5);
	glVertex2i(x, y);
	glTexCoord2d(0.25, 0.5);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawTailRight(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.75, 0.25);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(1.0, 0.25);
	glVertex2i(x, y + 1); 
	glTexCoord2d(1.0, 0.5);
	glVertex2i(x, y);
	glTexCoord2d(0.75, 0.5);
	glVertex2i(x + 1, y); 
	glEnd();
}

void drawVerticalBody(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.0, 0.75);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.25, 0.75);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.25, 1.0);
	glVertex2i(x, y);
	glTexCoord2d(0.0, 1.0);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawHorizontalBody(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.25, 0.75);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.5, 0.75);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.5, 1.0);
	glVertex2i(x, y);
	glTexCoord2d(0.25, 1.0);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawLowerRightBody(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.0, 0.5);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.25, 0.5);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.25, 0.75);
	glVertex2i(x, y);
	glTexCoord2d(0.0, 0.75);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawLowerLeftBody(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.75, 0.5);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(1.0, 0.5);
	glVertex2i(x, y + 1); 
	glTexCoord2d(1.0, 0.75);
	glVertex2i(x, y);
	glTexCoord2d(0.75, 0.75);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawUpperRightBody(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.25, 0.5);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.5, 0.5);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.5, 0.75);
	glVertex2i(x, y);
	glTexCoord2d(0.25, 0.75);
	glVertex2i(x + 1, y); 
	glEnd();
}
void drawUpperLeftBody(int x, int y){
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.5, 0.5);
	glVertex2i(x + 1, y + 1);
	glTexCoord2d(0.75, 0.5);
	glVertex2i(x, y + 1); 
	glTexCoord2d(0.75, 0.75);
	glVertex2i(x, y);
	glTexCoord2d(0.5, 0.75);
	glVertex2i(x + 1, y); 
	glEnd();
}

void generateGrass(){
	for (int x = 0; x < RANGE_X; x++){
		for (int y = 0; y < RANGE_Y; y++){
			if (((x >= 2 && x <= 7 || x <= RANGE_X - 3 && x >= RANGE_X - 8) && (y >= 2 && y <= 3 || y <= RANGE_Y - 3 && y >= RANGE_Y - 4)) || ((y >= 2 && y <= 7 || y <= RANGE_Y - 3 && y >= RANGE_Y - 8) && (x >= 2 && x <= 3 || x <= RANGE_X - 3 && x >= RANGE_X - 4))){
				grassPosition.insert(make_pair(x, y));
			}else if (((float)(RANGE_X - 1) / 2.0 - (float)x) * ((float)(RANGE_X - 1) / 2.0 - (float)x) + ((float)(RANGE_Y - 1) / 2.0 - (float)y) * ((float)(RANGE_Y - 1) / 2.0 - (float)y) <= 4.0){
				grassPosition.insert(make_pair(x, y));
			}
		}
	}
}
void drawGrass(){
	for (auto pair: grassPosition){
		int x = pair.first;
		int y = pair.second;
		glColor3f(1.0, 1.0, 1.0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
		glBegin(GL_POLYGON); 
		glTexCoord2d(0.75, 0.75);
		glVertex2i(x + 1, y + 1);
		glTexCoord2d(1.0, 0.75);
		glVertex2i(x, y + 1); 
		glTexCoord2d(1.0, 1.0);
		glVertex2i(x, y);
		glTexCoord2d(0.75, 1.0);
		glVertex2i(x + 1, y); 
		glEnd();
	}
}

void generateFood(int &x, int &y){
	bool overlap;
	srand(time(NULL));
	do {
		overlap = false;
		x = 0 + rand() % RANGE_X;
		y = 0 + rand() % RANGE_Y;
		for (int i = 0; i < snakeLength; i++){
			if (snakePositionX[i] == x && snakePositionY[i] == y){
				overlap = true;
				break;	
			}
		}
		if (!overlap && grassPosition.find(make_pair(x, y)) != grassPosition.end()){
			overlap = true;	
		}
	}while (overlap);
}
void drawFood(){
	if (ateFood) generateFood(foodPositionX, foodPositionY);
	ateFood = false;
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glBegin(GL_POLYGON); 
	glTexCoord2d(0.5, 0.75);
	glVertex2i(foodPositionX + 1, foodPositionY + 1);
	glTexCoord2d(0.75, 0.75);
	glVertex2i(foodPositionX, foodPositionY + 1); 
	glTexCoord2d(0.75, 1.0);
	glVertex2i(foodPositionX, foodPositionY);
	glTexCoord2d(0.5, 1.0);
	glVertex2i(foodPositionX + 1, foodPositionY); 
	glEnd();
	//glRectd(foodPositionX, foodPositionY, foodPositionX + 1, foodPositionY + 1);
	
}

void glut_reshape(int width, int height){
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (GLdouble)RANGE_X, 0.0, (GLdouble)RANGE_Y, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
}

void glut_timer(int t){
	glutPostRedisplay();
	glutTimerFunc(1000 / framePerSecond, glut_timer, 0);
}

void glut_keyboard(unsigned char key, int x, int y){
	switch(key){
	case 'q':
	case 'Q':
	case '\033':
		exit(0);
	}
}
void glut_navigate(int key, int, int){
	switch (key) {
    case GLUT_KEY_LEFT:
			if (snakeDirection != RIGHT) snakeDirection = LEFT;
			break;
    case GLUT_KEY_RIGHT:  
			if (snakeDirection != LEFT) snakeDirection = RIGHT;
			break;
    case GLUT_KEY_UP:
			if (snakeDirection != DOWN) snakeDirection = UP;
			break;
    case GLUT_KEY_DOWN:
			if (snakeDirection != UP) snakeDirection = DOWN;
			break;
  }
  glutPostRedisplay();
}

void set_texture(const char* inputFileNames){
	cv::Mat input = cv::imread(inputFileNames, 1);
	cv::cvtColor(input, input, CV_BGR2RGB);
	glBindTexture(GL_TEXTURE_2D, g_TextureHandles[0]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, input.cols, input.rows,	GL_RGB, GL_UNSIGNED_BYTE, input.data);
}