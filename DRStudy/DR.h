#pragma once
#include "stdafx.h"

class DR {
private:
	IKinectSensor * pSensor;
	IColorFrameSource* pColorFrameSource;
	IColorFrameReader* pColorFrameReader;
	IColorFrame* pColorFrame;
	IDepthFrameSource* pDepthFrameSource;
	IDepthFrameReader* pDepthFrameReader;
	IDepthFrame* pDepthFrame;
	ICoordinateMapper* pMapper;


	int imageWidth;
	int imageHeight;
	unsigned int bufferSize;
	cv::Mat bufferMat;
	cv::Mat colorMat;

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

	void initialize();
public:
	DR();
	~DR();

	void update();
};
