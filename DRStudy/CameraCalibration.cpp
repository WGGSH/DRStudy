#include "stdafx.h"
#include "CameraCalibration.h"

using namespace std;

CameraCalibration::CameraCalibration() {

}

CameraCalibration::~CameraCalibration() {

}

void CameraCalibration::update() {
	const int BOARD_W = 10;
	const int BOARD_H = 7;
	const cv::Size BOARD_SIZE = cv::Size(BOARD_W, BOARD_H);
	const int N_CORNERS = BOARD_W * BOARD_H;
	const int N_BOARDS = 10;
	const float SCALE = 20;
	const cv::Size IM_SIZE = cv::Size(640, 480);

	// 原画像を読み込む
	std::vector<cv::Mat> srcImage(N_BOARDS);
	for (int i = 0;i < N_BOARDS;i++) {
		srcImage[i] = cv::imread("../capture/cboard_rgb_" + std::to_string(i) + ".png");
		// srcImage[i] = cv::imread("../capture/cboard_kinect_" + std::to_string(i) + ".png");
	}

	// imagePointsの取得
	std::vector<std::vector<cv::Point2f>> imagePoints;
	std::vector<cv::Point2f> imageCorners;
	cv::Mat grayImage;
	bool found;
	for (int i = 0;i < N_BOARDS;i++) {
		// コーナーを検出する
		found = cv::findChessboardCorners(srcImage[i], BOARD_SIZE, imageCorners);
		// 精度を高める
		cv::cvtColor(srcImage[i], grayImage, CV_BGR2GRAY);
		cv::cornerSubPix(grayImage, imageCorners, cv::Size(9, 9), cv::Size(-1, -1), CvTermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 30, 0.1));
		// コーナーを描画する
		// ---
		// imagePointsに書き込む
		imagePoints.push_back(imageCorners);
	}

	// objectPointsを設定する
	std::vector<std::vector<cv::Point3f>> objectPoints;
	std::vector<cv::Point3f> objectCorners;
	for (int j = 0;j < BOARD_H;j++) {
		for (int i = 0;i < BOARD_W;i++) {
			objectCorners.push_back(cv::Point3f(i*SCALE, j*SCALE, 0.0f));
		}
	}
	
	for (int i = 0;i < N_BOARDS;i++) {
		objectPoints.push_back(objectCorners);
	}

	// カメラキャリブレーションを行う
	cv::Mat cameraMatrix;
	cv::Mat distCoeffs;
	std::vector<cv::Mat> rVecs;
	std::vector<cv::Mat> tVecs;
	double rms = cv::calibrateCamera(objectPoints, imagePoints, cv::Size(640, 480), cameraMatrix, distCoeffs, rVecs, tVecs);

	// キャリブレーション結果の表示
	cout << fixed << right;
	cout << "Reprojection Error(unit: pixel)" << endl;
	cout << "  " << rms << endl;
	cout << endl;
	cout << "cameraMatrix(unit: pixel)" << endl;
	cout << "  fx=" << cameraMatrix.at<double>(0, 0);
	cout << "  fy=" << cameraMatrix.at<double>(1, 1);
	cout << "  cx=" << cameraMatrix.at<double>(0, 2);
	cout << "  cy=" << cameraMatrix.at<double>(1, 2);
	cout << endl;
	cout << "distCoeffs" << endl;
	cout << "  k1=" << distCoeffs.at<double>(0, 0);
	cout << "  k2=" << distCoeffs.at<double>(0, 1);
	cout << "  p1=" << distCoeffs.at<double>(0, 2);
	cout << "  p2=" << distCoeffs.at<double>(0, 3);
	cout << "  k3=" << distCoeffs.at<double>(0, 4);
	cout << endl << endl;
	// キャリブレーション結果を格納する
	cv::FileStorage fs("../calibration/calibration_rgbcamera.xml", cv::FileStorage::WRITE);
	// cv::FileStorage fs("../calibration/calibration_kinect.xml", cv::FileStorage::WRITE);
	fs << "cameraMatrix" << cameraMatrix;
	fs << "distCoeffs" << distCoeffs;
	fs.release();
	
}