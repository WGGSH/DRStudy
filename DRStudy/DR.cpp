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
	this->safeRelease(this->pMapper);
}

void DR::initialize() {
	if (FAILED(GetDefaultKinectSensor(&this->pSensor))) {
		throw new exception("DefaultKinectSensor");
	}
	if (FAILED(this->pSensor->Open())) {
		throw new exception("IKinectSensor::Open()");
	}

	if (FAILED(this->pSensor->get_DepthFrameSource(&this->pDepthFrameSource))) {
		throw new exception("IKinectSensor::get_DepthFrameSource()");
	}

	if (FAILED(this->pSensor->get_ColorFrameSource(&this->pColorFrameSource))) {
		throw new exception("IKinectSensor::get_ColorFrameSource()");
	}
	if (FAILED(this->pSensor->get_CoordinateMapper(&this->pMapper))) {
		throw new exception("IKinectSensor::get_CoodinateMapper");
	}
	if (FAILED(this->pColorFrameSource->OpenReader(&this->pColorFrameReader))) {
		throw new exception("IColorFrameSource::OpenReader()");
	}

	if (FAILED(this->pDepthFrameSource->OpenReader(&this->pDepthFrameReader))) {
		throw new exception("IDepthFrameSource::OpenReader()");
	}



	this->imageWidth = 1920;
	this->imageHeight = 1080;
	this->bufferSize = this->imageWidth*this->imageHeight * 4 * sizeof(unsigned char);
	this->bufferMat = cv::Mat(this->imageHeight, this->imageWidth, CV_8UC4);
	this->colorMat = cv::Mat(this->imageHeight / 2, this->imageWidth / 2, CV_8UC4);
	cv::namedWindow("Color");

	this->depthWidth = 512;
	this->depthHeight = 424;
	this->depthBufferSize = this->depthWidth*this->depthHeight * sizeof(unsigned short);
	this->depthBufferMat = cv::Mat(this->depthHeight, this->depthWidth, CV_16UC1);
	this->depthMat = cv::Mat(this->depthHeight, this->depthWidth, CV_8UC1);
	cv::namedWindow("Depth");

	this->depthBuffer.resize(this->depthWidth*this->depthHeight);
}

void DR::update() {
	while (true) {
		// Color Frame
		this->pColorFrame = nullptr;
		if (SUCCEEDED(this->pColorFrameReader->AcquireLatestFrame(&this->pColorFrame)) &&
			SUCCEEDED(this->pColorFrame->CopyConvertedFrameDataToArray(this->bufferSize, reinterpret_cast<BYTE*>(this->bufferMat.data), ColorImageFormat_Bgra))) {
			cv::resize(this->bufferMat, this->colorMat, cv::Size(), 0.5, 0.5);
		}
		this->safeRelease(this->pColorFrame);

		// Show Window
		cv::imshow("Color", this->colorMat);


		// Depth Frame
		this->pDepthFrame = nullptr;
		/* if (SUCCEEDED(this->pDepthFrameReader->AcquireLatestFrame(&this->pDepthFrame)) &&
		SUCCEEDED(this->pDepthFrame->AccessUnderlyingBuffer(&this->depthBufferSize, reinterpret_cast<UINT16**>(&this->depthBufferMat.data)))) {
		this->depthBufferMat.convertTo(this->depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
		}
		this->safeRelease(this->pDepthFrame);
		// Show Window
		cv::imshow("Depth", this->depthMat);*/
		if (SUCCEEDED(this->pDepthFrameReader->AcquireLatestFrame(&this->pDepthFrame)) &&
			SUCCEEDED(this->pDepthFrame->AccessUnderlyingBuffer(&this->depthBufferSize, reinterpret_cast<UINT16**>(&this->depthBufferMat.data)))) {
			this->pDepthFrame->CopyFrameDataToArray(static_cast<UINT>(this->depthBuffer.size()), &this->depthBuffer[0]);
			this->depthBufferMat.convertTo(this->depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
			this->safeRelease(this->pDepthFrame);
			// Show Window
			cv::imshow("Depth", this->depthMat);
		}


		// RGBÇ∆ãóó£âÊëúÇÃëŒâû
		/*if (SUCCEEDED(this->pSensor->get_CoordinateMapper(&pMapper))) {
		UINT depthPointCount = this->depthWidth*this->depthHeight;
		UINT cameraPointCount = this->imageWidth*this->imageHeight;
		CameraSpacePoint* cameraSpacePoints = new CameraSpacePoint[this->imageWidth*this->imageHeight];
		if (SUCCEEDED(pMapper->MapColorFrameToCameraSpace(depthPointCount, reinterpret_cast<UINT16*>(this->depthMat.data), cameraPointCount, cameraSpacePoints))) {
		}
		delete cameraSpacePoints;
		this->safeRelease(pMapper);


		}*/

		// RGBÇ∆ãóó£âÊëúÇÃà íuçáÇÌÇπ

		// Depth Å® RGB Ç÷ÇÃëŒâû
		ColorSpacePoint* colorSpacePoints = new ColorSpacePoint[this->imageWidth*this->imageHeight];
		int s = this->depthHeight*this->depthWidth;
		// if (SUCCEEDED(pMapper->MapDepthFrameToColorSpace(s, reinterpret_cast<UINT16*>(this->depthBufferMat.data), this->depthWidth*this->depthHeight, colorSpacePoints))){
		if (SUCCEEDED(pMapper->MapDepthFrameToColorSpace(s, &this->depthBuffer[0], this->depthWidth*this->depthHeight, colorSpacePoints))) {

			std::vector<BYTE> buffer(this->depthWidth*this->depthHeight * 4);

			for (int depthY = 0;depthY < this->depthHeight;depthY++) {
				for (int depthX = 0;depthX < this->depthWidth;depthX++) {
					unsigned int depthIndex = depthY * this->depthWidth + depthX;
					const int colorX = static_cast<int>(colorSpacePoints[depthIndex].X + 0.5f);
					const int colorY = static_cast<int>(colorSpacePoints[depthIndex].Y + 0.5f);
					if ((0 <= colorX) && (colorX < this->imageWidth) && (0 <= colorY) && (colorY < this->imageHeight)) {
						const unsigned int colorIndex = (colorY*this->imageWidth + colorX) * 4;
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

		// RGB Å® Depth Ç÷ÇÃëŒâû
		std::vector<DepthSpacePoint> depthSpacePoints(this->imageWidth*this->imageHeight);
		if (SUCCEEDED(this->pMapper->MapColorFrameToDepthSpace(this->depthBufferSize, &this->depthBuffer[0], depthSpacePoints.size(), &depthSpacePoints[0]))) {
			std::vector<UINT16> buffer(this->imageWidth*this->imageHeight);

			for (int colorY = 0;colorY < this->imageHeight;colorY++) {
				for (int colorX = 0;colorX < this->imageWidth;colorX++) {
					unsigned int colorIndex = colorY * this->imageWidth + colorX;
					const int depthX = static_cast<int>(depthSpacePoints[colorIndex].X + 0.5f);
					const int depthY = static_cast<int>(depthSpacePoints[colorIndex].Y + 0.5f);
					if ((0 <= depthX) && (depthX < this->depthWidth) && (0 <= depthY) && (depthY < this->depthHeight)) {
						const unsigned int depthIndex = depthY * this->depthWidth + depthX;
						buffer[colorIndex] = depthBuffer[depthIndex] / 255.0f*800.0f * 4;
					}
				}
			}


			this->colorToDepthMat = cv::Mat(this->imageHeight, this->imageWidth, CV_16UC1, &buffer[0]).clone();
			cv::imshow("ColortoDepth", this->colorToDepthMat);

		}

		if (cv::waitKey(30) == VK_ESCAPE) {
			break;
		}
	}
}