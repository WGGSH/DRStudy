#pragma once

class DR;

class GL {
private:
	DR & dr;
	int count;

	int imageWidth;
	int imageHeight;
	CameraSpacePoint* cameraSpacePoints;
	ColorSpacePoint* colorSpacePoints;
	uchar* colorData;


public:
	GL(int argc, char** argv,DR& _dr);
	~GL();

	void setWindowSize(float _x, float _Y);
	

	static void display();
	static void reshape(int,int);
	static void onTimer(int);
	static void visibility(int);
	static void keyUp(unsigned char, int, int);

	void update();

	void draw();
};