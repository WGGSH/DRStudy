// DRStudy.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "DR.h"
#include "CameraCalibration.h"

using namespace std;

int main(int argc, char** argv)
{
	DR *dr=nullptr;
	try {
		// dr = new DR();
		dr = new DR(argc, argv);
		dr->update();
	}
	catch (exception &ex) {
		cout << ex.what() << endl;
	}

	if (dr != nullptr)delete dr;
	
	/*CameraCalibration* cameraCalibration = new CameraCalibration();
	cameraCalibration->update();
	delete cameraCalibration;
	*/

	return 0;
}

