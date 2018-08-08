#include "stdafx.h"

#include "stdafx.h"
#include "DR.h"

using namespace std;

// http://venuschjp.blogspot.com/2015/02/opencv.html
// http://opencv.jp/opencv-2svn/cpp/camera_calibration_and_3d_reconstruction.html

DR::DR() {
	this->initialize();
}

DR::DR(int _argc, char** _argv) :
	argc(_argc), argv(_argv) {
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
	
	this->rgbCameraInitialize();

	this->useKinect = true;
	this->useRGBCamera = true;
	this->quitFlag = false;

	// GL
	//this->gl = new GL(this->argc,this->argv,*this);
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
	// cv::imshow("Color", this->colorMat);
}

void DR::drawDepth() {
	// Depth Frame
	this->pDepthFrame = nullptr;
	if (SUCCEEDED(this->pDepthFrameReader->AcquireLatestFrame(&this->pDepthFrame)) ){
		if (SUCCEEDED(this->pDepthFrame->AccessUnderlyingBuffer(&this->depthBufferSize, reinterpret_cast<UINT16**>(&this->depthBufferMat.data)))) {
			this->pDepthFrame->CopyFrameDataToArray(static_cast<UINT>(this->depthBuffer.size()), &this->depthBuffer[0]);
			this->depthBufferMat.convertTo(this->depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
			this->safeRelease(this->pDepthFrame);
			// Show Window
			cv::imshow("Depth", this->depthMat);
		}
	}
}

void DR::drawColorToDepth() {
	// RGB → Depth への対応
	std::vector<DepthSpacePoint> depthSpacePoints(this->colorWidth*this->colorHeight);
	if (SUCCEEDED(this->pCoodinateMapper->MapColorFrameToDepthSpace(this->depthBufferSize, &this->depthBuffer[0], static_cast<UINT>(depthSpacePoints.size()), &depthSpacePoints[0]))) {
		std::vector<UINT16> buffer(this->colorWidth*this->colorHeight);

		for (int colorY = 0;colorY < this->colorHeight;colorY++) {
			for (int colorX = 0;colorX < this->colorWidth;colorX++) {
				unsigned int colorIndex = colorY * this->colorWidth + colorX;
				const int depthX = static_cast<int>(depthSpacePoints[colorIndex].X + 0.5f);
				const int depthY = static_cast<int>(depthSpacePoints[colorIndex].Y + 0.5f);
				if ((0 <= depthX) && (depthX < this->depthWidth) && (0 <= depthY) && (depthY < this->depthHeight)) {
					const unsigned int depthIndex = depthY * this->depthWidth + depthX;
					buffer[colorIndex] = static_cast<UINT16>(depthBuffer[depthIndex] / 255.0f*800.0f * 4); // 色が合わない
				}
			}
		}


		this->colorToDepthMat = cv::Mat(this->colorHeight, this->colorWidth, CV_16UC1, &buffer[0]).clone();
		cv::resize(this->colorToDepthMat, this->colorToDepthMat, cv::Size(), 0.5, 0.5);
		cv::imshow("ColortoDepth", this->colorToDepthMat);

	}
}

void DR::drawDepthToColor() {
	// RGBと距離画像の位置合わせ

	// Depth → RGB への対応
	/* if (this->colorSpacePoints!=NULL) {
		delete this->colorSpacePoints;
	}*/
	delete this->colorSpacePoints;
	/*ColorSpacePoint* */colorSpacePoints = new ColorSpacePoint[this->colorWidth*this->colorHeight];
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
	// delete colorSpacePoints;
}

void DR::drawPointCloud() {
	delete this->cameraSpacePoints;
	this->cameraSpacePoints = new CameraSpacePoint[this->colorWidth*this->colorHeight];
	this->pCoodinateMapper->MapColorFrameToCameraSpace(this->depthWidth*this->depthHeight, reinterpret_cast<UINT16*>(&this->depthBuffer),this->colorWidth*this->colorHeight ,cameraSpacePoints);
	// colorMat.data
	/* for (int i = 0;i < this->depthBufferSize;i++) {
		cout << cameraSpacePoints[i].X << "," << cameraSpacePoints[i].Y << "," << cameraSpacePoints[i].Z << endl;
		cout << static_cast<int>(this->colorMat.data[i * 3]) << "," << static_cast<int>(this->colorMat.data[i * 3 + 1]) << "," << static_cast<int>(this->colorMat.data[i * 3 + 2]) << endl << endl;
	}*/

	return;
}

bool DR::findChessCorners(cv::Mat image, std::vector<cv::Point2f> *corners, cv::Size patternSize, cv::Size chessSize) {
	bool isFind = cv::findChessboardCorners(image, patternSize, *corners);

	if (isFind != true) {
		return false;
	}
	cv::Mat gray(image.rows, image.cols, CV_8UC1);
	cv::cvtColor(image, gray, CV_BGR2GRAY);
	cv::cornerSubPix(gray, *corners, chessSize, cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.01));
	
	return true;
}

void DR::keyInput() {
	int keyCode=cv::waitKey(30);
	switch (keyCode) {
	case -1:
		break;
	case '1': // Kinectの撮影
		cv::imwrite("../capture/cboard_kinect_" + std::to_string(this->writeCount++) + ".png", this->colorMat);
		break;
	case '2': // RGBカメラの撮影
		cv::imwrite("../capture/cboard_rgb_" + std::to_string(this->writeCount++) + ".png", this->cameraFrame);
		break;
	case 'k': // Kinectのスイッチ
	case 'K':
		this->useKinect = !this->useKinect;
		break;
	case 'r': // RGBカメラのスイッチ
	case 'R':
		this->useRGBCamera = !this->useRGBCamera;
		break;
	case 'q': //終了
	case 'Q':
		this->quitFlag = true;
		break;
	default:
		break;
	}

}

void DR::rgbCameraInitialize() {
	this->videoCapture = cv::VideoCapture(0);
	if (!this->videoCapture.isOpened()) {
		// カメラデバイスの起動失敗
		cout << "Failed: VideoCapture::Open" << endl;
	}
}

void DR::rgbCameraUpdate() {
	this->videoCapture >> this->cameraFrame;

	const int MARKER_SIZE = 20; //20mm

	std::vector<cv::Point2f>srcCorners;
	// this->findChessCorners(this->cameraFrame, &srcCorners, cv::Size(10, 7), cv::Size(20, 20));
	// cv::drawChessboardCorners(this->cameraFrame, cv::Size(10, 7), (cv::Mat)srcCorners, true);
	
	const int MARKER_WIDTH = 14;
	const int MARKER_HIGHT = 10;
	bool success = cv::findChessboardCorners(this->cameraFrame, cv::Size(MARKER_WIDTH,MARKER_HIGHT), srcCorners,CV_CALIB_CB_FAST_CHECK);

	if (success == true) {
		/*for (int i = 0;i < 70; i++) {
			cout << "(" << static_cast<int>(srcCorners[i].x) << "," << static_cast<int>(srcCorners[i].y) << ")";
			if (i % 10 == 9)cout << endl;
		}
		cout << endl << endl;*/

		

		std::vector<cv::Point3f>objCorners;
		for (int i = 0;i < srcCorners.size();i++) {
			objCorners.push_back(cv::Point3f((i % MARKER_WIDTH)*MARKER_SIZE, (i / MARKER_WIDTH)*MARKER_SIZE, 0));
		}

		cv::Mat cameraMatrix = cv::Mat(3, 3, CV_32F);
		/*cameraMatrix.at<float>(0, 0) = 551;
		cameraMatrix.at<float>(0, 2) = 302;
		cameraMatrix.at<float>(1, 1) = 551;
		cameraMatrix.at<float>(1, 2) = 194;
		cameraMatrix.at<float>(2, 2) = 1;

		cv::Mat distCoeffs = cv::Mat(1, 5, CV_32F);
		distCoeffs.at<float>(0, 0) = 0.10567f;
		distCoeffs.at<float>(0, 1) = -0.22032f;
		distCoeffs.at<float>(0, 2) = -0.16265f;
		distCoeffs.at<float>(0, 3) = -0.00596006f;
		distCoeffs.at<float>(0, 4) = -0.161604f;*/

		cameraMatrix.at<float>(0, 0) = 550.09f;
		cameraMatrix.at<float>(0, 2) = 309.994f;
		cameraMatrix.at<float>(1, 1) = 558.918f;
		cameraMatrix.at<float>(1, 2) = 188.667f;
		cameraMatrix.at<float>(2, 2) = 1;

		cv::Mat distCoeffs = cv::Mat(1, 5, CV_32F);
		distCoeffs.at<float>(0, 0) = 0.216923f;
		distCoeffs.at<float>(0, 1) = -0.131005f;
		distCoeffs.at<float>(0, 2) = -0.0345925f;
		distCoeffs.at<float>(0, 3) = -0.00363356f;
		distCoeffs.at<float>(0, 4) = -0.272545f;

		cv::Mat rVec;
		cv::Mat tVec;

		cv::solvePnP(objCorners, srcCorners, cameraMatrix, distCoeffs, rVec, tVec);

		// 推定結果を表示
		// cout << "rVec:" << endl << rVec << ", tVec:" << endl << tVec << endl << endl;

		cv::Mat rMat;
		cv::Rodrigues(rVec, rMat);
		cv::Mat worldMat = cv::Mat(4, 4, CV_32F);
		for (int y = 0;y < 3;y++) {
			for (int x = 0;x < 3;x++) {
				worldMat.at<float>(y, x) = rMat.at<float>(y, x);
			}
		}
		for (int y = 0;y < 3;y++) {
			worldMat.at<float>(y, 3) = tVec.at<float>(y, 0);
		}
		for (int x = 0;x < 3;x++) {
			worldMat.at<float>(3, x) = 0;
		}
		worldMat.at < float>(3, 3) = 1;

		cout << "mat:" << endl << worldMat << endl << endl;


		std::vector<cv::Point2f > dstPoints;
		cv::projectPoints(objCorners, rVec, tVec, cameraMatrix, distCoeffs,dstPoints);
		for (int x = 0;x < dstPoints.size();x++) {
			// cout << dstPoints[x]<< endl;
			cv::circle(this->cameraFrame, cv::Point(dstPoints[x].x, dstPoints[x].y), 10, cv::Scalar(255, 0, 0));
			// cv::circle(this->cameraFrame, cv::Point(dstPoints.at<cv::Vec2f>(x, 0)[0], dstPoints.at<cv::Vec2f>(x, 0)[1]), 10, cv::Scalar(255, 255, 255), 1, 1);
		}
	}

	cv::Mat rVec, tVec;


	cv::imshow("RGB-Camera", this->cameraFrame);

	/*int keyCode = cv::waitKey(30);
	if (keyCode == 's') {
		cv::imwrite("../capture/cboard_rgb_" + std::to_string(this->writeCount++) + ".png", this->cameraFrame);
	}*/

	this->cameraFrame.release();
}

void DR::kinectUpdate() {
	this->drawColor();
	// this->drawDepth();
	// this->drawPointCloud();

	// this->drawColorToDepth();
	// this->drawDepthToColor();


	// KinectのRGB画像からチェスボードの交点を求める
	std::vector<cv::Point2f> srcCorners;
	// this->findChessCorners(this->colorMat,&srcCorners, cv::Size(10, 7), cv::Size(20, 20),CALIB_

	const int MARKER_WIDTH = 14;
	const int MARKER_HIGHT = 10;
	bool success = cv::findChessboardCorners(this->colorMat, cv::Size(MARKER_WIDTH, MARKER_HIGHT), srcCorners, CV_CALIB_CB_FAST_CHECK);

	if (success == true) {
		
		std::vector<cv::Point3f> objCorners;
		for (int i = 0;i < srcCorners.size();i++) {
			objCorners.push_back(cv::Point3f((i % MARKER_WIDTH) * 20, (i / MARKER_WIDTH) * 20, 0));
		}

		cv::Mat cameraMatrix = cv::Mat(3, 3, CV_32F);
		cameraMatrix.at<float>(0, 0) = 671.579f; // fx
		cameraMatrix.at<float>(0, 2) = 492.771f; // cx
		cameraMatrix.at<float>(1, 1) = 621.429f; // fy
		cameraMatrix.at<float>(1, 2) = 67.7487f; // cy
		cameraMatrix.at<float>(2, 2) = 1; // 1

		cv::Mat distCoeffs = cv::Mat(1, 5, CV_32F);
		distCoeffs.at<float>(0, 0) = 0.0301921f;
		distCoeffs.at<float>(0, 1) = 0.583972f;
		distCoeffs.at<float>(0, 2) = -0.0652484f;
		distCoeffs.at<float>(0, 3) = -0.0652484f;
		distCoeffs.at<float>(0, 4) = -0.756578f;

		cv::Mat rVec, tVec;
		cv::solvePnP(objCorners, srcCorners, cameraMatrix, distCoeffs, rVec, tVec);

		std::vector<cv::Point2f> dstPoints;
		cv::projectPoints(objCorners, rVec, tVec, cameraMatrix, distCoeffs, dstPoints);
		for (int i = 0;i < dstPoints.size();i++) {
			cv::circle(this->colorMat, cv::Point(dstPoints[i].x, dstPoints[i].y), 10, cv::Scalar(255, 0, 0));
		}
	}

	

	// cv::drawChessboardCorners(this->colorMat, cv::Size(10, 7), (cv::Mat)srcCorners, true);
	cv::imshow("ChessBoard", this->colorMat);
}

void DR::update() {
	cout << "キー入力:" << endl;
	cout << "r: RGBカメラの使用切り替え" << endl;
	cout << "k: Kinectの使用切り替え" << endl;
	cout << "q: 終了" << endl;
	cout << endl;

	while (this->quitFlag==false) {
		
		if (this->useKinect)this->kinectUpdate();

		if (this->useRGBCamera)this->rgbCameraUpdate();

		//this->gl->update();
		
		// キー入力の取得
		this->keyInput();
	}
}