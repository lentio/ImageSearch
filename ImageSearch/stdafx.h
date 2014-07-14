// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "targetver.h"

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdarg.h>
#include <io.h>
#include <direct.h>
#include <conio.h>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

// boost头文件
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

// OpenCV头文件
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/nonfree/nonfree.hpp>  // SURF is nonfree

// 声明命名空间
using namespace std;
using namespace cv;

// TODO: reference additional headers your program requires here
#define OF_BOW_MAX_CLASSES    100
#define OF_BOW_MAX_Fold       10

#define DEFAULT_DATA_PATH        "./data"
#define DEFAULT_DATA_RAW_PATH    "\\raw\\"
#define DEFAULT_IMAGE_PATH       "./data/predict"
#define DEFAULT_DATA_FILE_ANNO   "./data/annotations.csv"
#define DEFAULT_DATA_ANNO_COL    4

#define IS_IMAGE_NORMAL_SIZE     400

#define IS_IMAGE_MIN_ROWS        2048
#define IS_IMAGE_MIN_COLS        1536

#define IS_DUMP_RESULT_FILE     "result.txt"
#define IS_DUMP_PREDICT_IMAGE   "/image.jpg"


