#include "stdafx.h"

#include "stdafx.h"
#include "DR.h"

using namespace std;

DR::DR() {
	this->initialize();
}

DR::~DR() {
	this->safeRelease(this->pSensor);
	this->safeRelease(this->pColorFrameSource);
	this->safeRelease(this->pColorFrameReader);
	this->safeRelease(this->pColorFrame);
	this->safeRelease(this->pDepthFrameSource);
	this->safeRelease(this->pDepthFrameReader);
	this->safeRelease(this->pDepthFrame);
	this->safeRelease(this->pCoodinateMapper);
}

void DR::initialize() {

	this->sensorInitialize();
	this->colorInitialize();
	this->depthInitialize();



	this->depthBuffer.resize(this->depthWidth*this->depthHeight);
}

void DR::sensorInitialize() {
	// 各種センサーの初期化
	ERROR_CHECK(GetDefaultKinectSensor(&this->pSensor));
	ERROR_CHECK(this->pSensor->Open());
	ERROR_CHECK(this->pSensor->get_DepthFrameSource(&this->pDepthFrameSource));
	ERROR_CHECK(this->pSensor->get_ColorFrameSource(&this->pColorFrameSource));
	ERROR_CHECK(this->pSensor->get_CoordinateMapper(&this->pCoodinateMapper));
	ERROR_CHECK(this->pColorFrameSource->OpenReader(&this->pColorFrameReader));
	ERROR_CHECK(this->pDepthFrameSource->OpenReader(&this->pDepthFrameReader));
}

void DR::colorInitialize() {

	// RGB画像の初期設定
	this->colorWidth = 1920;
	this->colorHeight = 1080;
	this->bufferSize = this->colorWidth*this->colorHeight * 4 * sizeof(unsigned char);
	this->bufferMat = cv::Mat(this->colorHeight, this->colorWidth, CV_8UC4);
	this->colorMat = cv::Mat(this->colorHeight / 2, this->colorWidth / 2, CV_8UC4);
	cv::namedWindow("Color");
}

void DR::depthInitialize() {

	// Depth画像の初期設定
	this->depthWidth = 512;
	this->depthHeight = 424;
	this->depthBufferSize = this->depthWidth*this->depthHeight * sizeof(unsigned short);
	this->depthBufferMat = cv::Mat(this->depthHeight, this->depthWidth, CV_16UC1);
	this->depthMat = cv::Mat(this->depthHeight, this->depthWidth, CV_8UC1);
	cv::namedWindow("Depth");
}

void DR::drawColor() {
	// Color Frame
	this->pColorFrame = nullptr;
	if (SUCCEEDED(this->pColorFrameReader->AcquireLatestFrame(&this->pColorFrame)) &&
		SUCCEEDED(this->pColorFrame->CopyConvertedFrameDataToArray(this->bufferSize, reinterpret_cast<BYTE*>(this->bufferMat.data), ColorImageFormat_Bgra))) {
		cv::resize(this->bufferMat, this->colorMat, cv::Size(), 0.5, 0.5);
	}
	this->safeRelease(this->pColorFrame);

	// Show Window
	cv::imshow("Color", this->colorMat);
}

void DR::drawDepth() {
	// Depth Frame
	this->pDepthFrame = nullptr;
	if (SUCCEEDED(this->pDepthFrameReader->AcquireLatestFrame(&this->pDepthFrame)) &&
		SUCCEEDED(this->pDepthFrame->AccessUnderlyingBuffer(&this->depthBufferSize, reinterpret_cast<UINT16**>(&this->depthBufferMat.data)))) {
		this->pDepthFrame->CopyFrameDataToArray(static_cast<UINT>(this->depthBuffer.size()), &this->depthBuffer[0]);
		this->depthBufferMat.convertTo(this->depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
		this->safeRelease(this->pDepthFrame);
		// Show Window
		cv::imshow("Depth", this->depthMat);
	}
}

void DR::update() {
	while (true) {
		
		this->drawColor();
		this->drawDepth();

		


		// RGBと距離画像の対応
		/*if (SUCCEEDED(this->pSensor->get_CoordinateMapper(&pCoodinateMapper))) {
		UINT depthPointCount = this->depthWidth*this->depthHeight;
		UINT cameraPointCount = this->colorWidth*this->colorHeight;
		CameraSpacePoint* cameraSpacePoints = new CameraSpacePoint[this->colorWidth*this->colorHeight];
		if (SUCCEEDED(pCoodinateMapper->MapColorFrameToCameraSpace(depthPointCount, reinterpret_cast<UINT16*>(this->depthMat.data), cameraPointCount, cameraSpacePoints))) {
		}
		delete cameraSpacePoints;
		this->safeRelease(pCoodinateMapper);


		}*/

		// RGBと距離画像の位置合わせ

		// Depth → RGB への対応
		ColorSpacePoint* colorSpacePoints = new ColorSpacePoint[this->colorWidth*this->colorHeight];
		int s = this->depthHeight*this->depthWidth;
		// if (SUCCEEDED(pCoodinateMapper->MapDepthFrameToColorSpace(s, reinterpret_cast<UINT16*>(this->depthBufferMat.data), this->depthWidth*this->depthHeight, colorSpacePoints))){
		if (SUCCEEDED(pCoodinateMapper->MapDepthFrameToColorSpace(s, &this->depthBuffer[0], this->depthWidth*this->depthHeight, colorSpacePoints))) {

			std::vector<BYTE> buffer(this->depthWidth*this->depthHeight * 4);

			for (int depthY = 0;depthY < this->depthHeight;depthY++) {
				for (int depthX = 0;depthX < this->depthWidth;depthX++) {
					unsigned int depthIndex = depthY * this->depthWidth + depthX;
					const int colorX = static_cast<int>(colorSpacePoints[depthIndex].X + 0.5f);
					const int colorY = static_cast<int>(colorSpacePoints[depthIndex].Y + 0.5f);
					if ((0 <= colorX) && (colorX < this->colorWidth) && (0 <= colorY) && (colorY < this->colorHeight)) {
						const unsigned int colorIndex = (colorY*this->colorWidth + colorX) * 4;
						depthIndex *= 4;
						buffer[depthIndex + 0] = this->bufferMat.data[colorIndex + 0];
						buffer[depthIndex + 1] = this->bufferMat.data[colorIndex + 1];
						buffer[depthIndex + 2] = this->bufferMat.data[colorIndex + 2];
						buffer[depthIndex + 3] = this->bufferMat.data[colorIndex + 3];
					}
				}
			}

			this->depthToColorMat = cv::Mat(this->depthHeight, this->depthWidth, CV_8UC4, &buffer[0]);
			cv::imshow("DepthtoColor", this->depthToColorMat);

		}
		delete colorSpacePoints;

		// RGB → Depth への対応
		std::vector<DepthSpacePoint> depthSpacePoints(this->colorWidth*this->colorHeight);
		if (SUCCEEDED(this->pCoodinateMapper->MapColorFrameToDepthSpace(this->depthBufferSize, &this->depthBuffer[0], depthSpacePoints.size(), &depthSpacePoints[0]))) {
			std::vector<UINT16> buffer(this->colorWidth*this->colorHeight);

			for (int colorY = 0;colorY < this->colorHeight;colorY++) {
				for (int colorX = 0;colorX < this->colorWidth;colorX++) {
					unsigned int colorIndex = colorY * this->colorWidth + colorX;
					const int depthX = static_cast<int>(depthSpacePoints[colorIndex].X + 0.5f);
					const int depthY = static_cast<int>(depthSpacePoints[colorIndex].Y + 0.5f);
					if ((0 <= depthX) && (depthX < this->depthWidth) && (0 <= depthY) && (depthY < this->depthHeight)) {
						const unsigned int depthIndex = depthY * this->depthWidth + depthX;
						buffer[colorIndex] = depthBuffer[depthIndex] / 255.0f*800.0f * 4; // 色が合わない
					}
				}
			}


			this->colorToDepthMat = cv::Mat(this->colorHeight, this->colorWidth, CV_16UC1, &buffer[0]).clone();
			cv::resize(this->colorToDepthMat, this->colorToDepthMat, cv::Size(), 0.5, 0.5);
			cv::imshow("ColortoDepth", this->colorToDepthMat);

		}

		if (cv::waitKey(30) == VK_ESCAPE) {
			break;
		}
	}
}