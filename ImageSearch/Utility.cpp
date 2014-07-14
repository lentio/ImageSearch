#include "stdafx.h"
#include "Utility.h"

CUtility::CUtility()
{
}


CUtility::~CUtility()
{
}

void CUtility::debug(char* format, ...)
{
    char buf[2048] = { 0 };

    va_list args;
    va_start(args, format);
    _vsnprintf(buf, sizeof(buf)-1, format, args);
    va_end(args);

    //time_t now_time;
    //now_time = time(NULL);

    //struct tm *local;
    //local = localtime(&now_time);  //获取当前系统时间  
    //cout << asctime(local);

    cout << "<" << CUtility::showTime() << ">[debug] " << buf << endl;
    OutputDebugStringA(buf);
}

Mat CUtility::normalSize(Mat img)
{
    // scale into 400 x ...
    Size dsize = Size(IS_IMAGE_NORMAL_SIZE, (img.rows * IS_IMAGE_NORMAL_SIZE / img.cols));
    Mat scaleImage = Mat(dsize, CV_32S);
    resize(img, scaleImage, dsize);
    return scaleImage;
}

bool CUtility::getPathFile(string szPathFile, string &szFilePath, string &szFileName)
{
    assert(szPathFile.size() > 0);
    for (int i = 0; i < szPathFile.size(); ++i){
        if (szPathFile[i] == '\\') {
            szPathFile[i] = '/';
        }
    }

    int nPos = szPathFile.rfind('/');
    if (nPos < 0){ // no found path, only return file name;
        szFileName = szPathFile;
        szFilePath = "";
    }
    else if (nPos == szPathFile.size()){ // at last, only return path
        szPathFile = szPathFile.substr(0, szPathFile.size() - 1);
        szFileName = "";
    }
    else{
        szFileName = szPathFile.substr(nPos + 1, szPathFile.size());
        szFilePath = szPathFile.substr(0, nPos);
    }

    return true;
}

string CUtility::getCwd()
{
    char *buffer;
    if ((buffer = _getcwd(NULL, 0)) == NULL){
        assert(0);
    }

    return buffer;
}

string CUtility::showTime()
{
    time_t now_time;
    now_time = time(NULL);
    struct tm local;
    localtime_s(&local, &now_time);  //获取当前系统时间  

    //ostringstream ostr; 
    //ostr << local.tm_yday + 1900 << "-" << local.tm_mon << "-" << local.tm_mday << endl;
    char curTime[32] = { 0, };
    sprintf_s(curTime, "%04d/%02d/%02d %02d:%02d:%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
    return curTime;
}