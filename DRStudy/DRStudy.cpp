// DRStudy.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "DR.h"

using namespace std;

int main()
{
	DR *dr=nullptr;
	try {
		dr = new DR();
		dr->update();
	}
	catch (exception &ex) {
		cout << ex.what() << endl;
	}

	if (dr != nullptr)delete dr;
	return 0;
}

