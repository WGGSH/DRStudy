#include  "stdafx.h"
#include "GL.h"

#include "DR.h"

using namespace std;

GL::GL(int argc, char** argv,DR& _dr) :dr(_dr){
	this->count = 0;

	glutInit(&argc,argv);
	glutInitWindowSize(640, 480);
	glutCreateWindow(0);


	// ウィンドウ全体をビューポートにする
	glViewport(0, 0, 640, 480);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(GL::display);
	glutReshapeFunc(GL::reshape);
	glutVisibilityFunc(GL::visibility);
	glutIgnoreKeyRepeat(GL_TRUE);
	// glutKeyboardFunc(GL::keyDown);
	glutKeyboardUpFunc(GL::keyUp);
	
	glutTimerFunc(100, GL::onTimer, 0);

	glClearColor(0, 0, 0, 1);

	// glutMainLoop();

}

GL::~GL() {

}

void GL::display() {

}

void GL::reshape(int x, int y) {

}

void GL::visibility(int visible) {

}

void GL::keyUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		break;
	case 'd':
		break;
	case 'w':
		break;
	case 's':
		break;
	default:
		break;
	}
}

void GL::onTimer(int time) {
	static int count = 0;
	count++;
	float length = 10.0f;
	/*gluLookAt(cos(count/180.0f)*length, length, sin(180.0f)*length,
		0, 0, 0,
		0.0f, 0.5f, 0.0f);*/

	glutTimerFunc(160, GL::onTimer, 0);
}

void GL::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gluLookAt(1, 0, 0, 0, 0, 0, 0, 1, 0);

	this->imageWidth = this->dr.getImageWidth();
	this->imageHeight = this->dr.getImageHeight();
	this->cameraSpacePoints = this->dr.getCameraSpacePoints();
	this->colorSpacePoints = this->dr.getColorSpacePoints();
	this->colorData = this->dr.getColorData();

	GLfloat vtx1[][3] = {
		{0,0,0},
	{0.5,0.5,0},
	{1,1,0}
	};

	const int indexX = 1;
	const int indexY = this->imageHeight;
	const int indexColorX = 3;
	const int indexColorY = this->imageHeight * 3;

	glBegin(GL_POINTS);
	for (int y = 0;y < this->imageHeight - 1;y++) {
		for(int x=0;x<this->imageWidth-1;x++){
			int index = (y*this->imageWidth) + x;
			int indexColor = index * 3;
			glColor3i(255, 0, 0);
			float px = this->cameraSpacePoints[index].X;
			float py = this->cameraSpacePoints[index].Y;
			float pz = this->cameraSpacePoints[index].Z;
			glVertex3f(px, py, pz);
			/* glVertex3f(this->cameraSpacePoints[index].X, this->cameraSpacePoints[index].Y, this->cameraSpacePoints[index].Z);
			glVertex3f(this->cameraSpacePoints[index+indexX].X, this->cameraSpacePoints[index+indexX].Y, this->cameraSpacePoints[index+indexX].Z);
			glVertex3f(this->cameraSpacePoints[index + indexY].X, this->cameraSpacePoints[index + indexY].Y, this->cameraSpacePoints[index + indexY].Z);*/
			
		}
	}
	glEnd();

	glutSwapBuffers();
}

void GL::update() {
	this->count++;
	glutMainLoopEvent();
	this->draw();
}