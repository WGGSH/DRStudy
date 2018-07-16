#pragma once
#include "stdafx.h"

class DR {
private:
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
	cv::Mat depthMat;

	cv::Mat depthToColorMat;
	std::vector<UINT16> depthBuffer;

	cv::Mat colorToDepthMat;


	template<class Interface>
	inline void safeRelease(Interface *& pInterfaceToRelease) {
		if (pInterfaceToRelease != nullptr) {
			pInterfaceToRelease->Release();
			pInterfaceToRelease = nullptr;
		}
	}

	void sensorInitialize();
	void colorInitialize();
	void depthInitialize();

	void drawColor();
	void drawDepth();

	void initialize();
public:
	DR();
	~DR();

	void update();
};
