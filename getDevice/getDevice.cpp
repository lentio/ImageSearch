// getDevice.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
    if (argc < 2) {
        cerr << "²ÎÊý´íÎó" << endl;
        cerr << "usage:" << endl;
        cerr << "getDevice <usb mount path for adb> [delay_millseconds]" << endl;
        system("pause");
        exit(__LINE__);
    }

    int nMillSeconds = 1000;
    if (argc > 2){
        nMillSeconds = atoi(argv[2]);
    }

    if (nMillSeconds < 100 || nMillSeconds > 10000) {
        nMillSeconds = 1000;
    }

    string szPull = "adb pull ";
    szPull += argv[1];

    string szRm = "adb shell \"rm ";
    szRm += argv[1];
    szRm += "/*.jpg\"";

    cout << "excute:" << endl;
    cout << szPull << endl;
    cout << szRm << endl;
    cout << "delay millseconds: " << nMillSeconds << endl;

    while (1){
        //cout << szPull << endl;
        system(szPull.c_str());
        //cout << szRm << endl;
        system(szRm.c_str());
        Sleep(nMillSeconds);
    }

    system("pause");
	return 0;
}

