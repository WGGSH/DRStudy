#pragma once
#include "GL.h"

class MyGL {
private:
	static GL *gl;
public:
	MyGL();
	~MyGL();

	static void display();
	static void reshape(int, int);
	static void idle();
	static void onTimer(int);
	static void visibility(int);

	static void initialize(int, char**);
	static void update();
	static void terminate();
};