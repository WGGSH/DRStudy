#pragma once
#include "stdafx.h"
#include "GL.h"

class DR {
private:
	int argc;
	char** argv;


	// Interface
	IKinectSensor * pSensor;
	IColorFrameSource* pColorFrameSource;
	IColorFrameReader* pColorFrameReader;
	IColorFrame* pColorFrame;
	IDepthFrameSource* pDepthFrameSource;
	IDepthFrameReader* pDepthFrameReader;
	IDepthFrame* pDepthFrame;
	ICoordinateMapper* pCoodinateMapper;


	// Color
	int colorWidth;
	int colorHeight;
	unsigned int bufferSize;
	cv::Mat bufferMat;
	cv::Mat colorMat;

	// Depth
	int depthWidth;
	int depthHeight;
	unsigned int depthBufferSize;
	cv::Mat depthBufferMat;
	std::vector<UINT16> depthBuffer;
	cv::Mat depthMat;

	ColorSpacePoint*  colorSpacePoints;
	cv::Mat depthToColorMat;
	cv::Mat colorToDepthMat;

	// RGB�J�����֘A
	cv::VideoCapture videoCapture; // �J�����f�o�C�X
	cv::Mat cameraFrame; // �J�����摜

	// �e��t���O�֘A
	bool useKinect;
	bool useRGBCamera;
	bool quitFlag;
	int writeCount; // �J�����摜�̕ۑ�����

	CameraSpacePoint* cameraSpacePoints;

	// OpenGL�֘A
	GL *gl;


	template<class Interface>
	inline void safeRelease(Interface *& pInterfaceToRelease) {
		if (pInterfaceToRelease != nullptr) {
			pInterfaceToRelease->Release();
			pInterfaceToRelease = nullptr;
		}
	}

	// Kinect�֘A
	void sensorInitialize();
	void colorInitialize();
	void depthInitialize();

	void drawColor();
	void drawDepth();
	void drawColorToDepth();
	void drawDepthToColor();
	void drawPointCloud();

	void kinectUpdate();

	bool findChessCorners(cv::Mat image, std::vector<cv::Point2f> *corners, cv::Size patternSize, cv::Size chessSize);

	// RGB�J�����֘A
	void rgbCameraInitialize();
	
	void keyInput();

	void initialize();

	void rgbCameraUpdate();
public:
	DR();
	DR(int,char**);
	~DR();

	void update();

	int getImageWidth() { return this->colorWidth; }
	int getImageHeight() { return this->colorHeight; }
	ColorSpacePoint* getColorSpacePoints() { return this->colorSpacePoints; }
	CameraSpacePoint* getCameraSpacePoints() { return this->cameraSpacePoints; }
	uchar* getColorData() { return this->colorMat.data; }
};
