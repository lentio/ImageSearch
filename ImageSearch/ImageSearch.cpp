// ImageSearch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ObjectFeature.h"
#include "Utility.h"

int g_nMaxObjectID = 0;

bool LoadImages(string cvsfile, vector<int> &nImageId, vector<string> &szImageName, vector<string> &szImageRawList, vector<string> &szImageUrl)
{
    ifstream ifs(cvsfile, ifstream::in);
    if (!ifs.good()) {
        cerr << boost::format("不能读取图像文件列表[%s]") % cvsfile << endl;
        exit(__LINE__);
    }

    while (ifs.good()) {
        string t;
        ifs >> t;

        vector<string> fields;
        boost::split(fields, t, boost::is_any_of(";"));
        if (fields.size() < DEFAULT_DATA_ANNO_COL) {
            break;
        }

        if (fields[0].size()) {
            int nId = atoi(fields[0].c_str());
            nImageId.push_back(nId); 
            if (nId > g_nMaxObjectID){
                g_nMaxObjectID = nId;
            }
        }

        if (fields[1].size()) {
            szImageName.push_back(fields[1]);
        }

        if (fields[2].size()) {
            szImageRawList.push_back(fields[2]);
        }

        if (fields[3].size()) {
            szImageUrl.push_back(fields[3]);
        }

        //cout << fields[0] << ": " << fields[1] << ": " << fields[2] << endl;
    }

    ifs.close();

    //cout << "max bow classes: " << g_nMaxBowID << endl;
    return true;
}

bool test_detect()
{
    CObjectFeature objectFeature;

    // add image
    vector<int> nImageIdList;
    vector<string> szImageNameList;
    vector<string> szImageUrlList;
    vector<string> szImageRawList;
    
    LoadImages(DEFAULT_DATA_FILE_ANNO, nImageIdList, szImageNameList, szImageRawList, szImageUrlList);

    /*
    for (int i = 0; i < nImageIdList.size(); ++i) {
        objectFeature.add_image((char *)szImageNameList[i].c_str(), nImageIdList[i]);
    }

    cout << "add images: " << nImageIdList.size() << endl;

    // extract feathures
    objectFeature.extract_features(DEFAULT_DATA_PATH);

    // training images
    objectFeature.train(DEFAULT_DATA_PATH);
    */

    // get a live image
    // 打开摄像头
    VideoCapture camera;
    camera.open(0);
    if (!camera.isOpened()) {
        cerr << "打开摄像头失败" << endl;
        exit(__LINE__);
    }

    camera.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    camera.set(CV_CAP_PROP_FPS, 30);

    cout << "Open Camera and Press 'd' to get a image" << endl;

    // loop get image from camera
    char key_press = 0;
    while (1)
    {
        key_press = (char)waitKey(100);

        Mat buf;
        Mat t = Mat::zeros(480, 640, CV_8UC3);
        camera >> t;
        t.copyTo(buf);

        // 显示
        imshow("test", buf);

        if (key_press == 27) {
            break;
        }
        else if (key_press == 'd') {
            // save mat into local file
            string szSavePredictFile = DEFAULT_DATA_PATH;
            szSavePredictFile += "/predict.jpg";
            imwrite(szSavePredictFile, buf);

            // predict
            int nObjectId = objectFeature.predict(DEFAULT_DATA_PATH, (char *)szSavePredictFile.c_str());
            cout << "Predict image id is " << nObjectId << " at " << szSavePredictFile << endl;

            // show image
            assert(nObjectId <= g_nMaxObjectID);
            string szPredictFile = szImageUrlList[nObjectId-1];

            //读入图像
            Mat img = imread(szPredictFile, CV_LOAD_IMAGE_COLOR);
            if (img.empty()) {
                cout << "Could not open or find the image: " << szPredictFile << endl;
                return false;
            }

            //创建窗口
            namedWindow("predict", CV_WINDOW_AUTOSIZE);

            //显示图像
            imshow("predict", img);
        }
    }

    return true;
}

bool verifySamples(CObjectFeature &objectFeature)
{
    // add image
    vector<int> nImageIdList;
    vector<string> szImageNameList;
    vector<string> szImageUrlList;
    vector<string> szImageRawList;
    LoadImages(DEFAULT_DATA_FILE_ANNO, nImageIdList, szImageNameList, szImageRawList, szImageUrlList);

    int nFail = 0, nTotle = 0;
    for (int i = 0; i < szImageNameList.size(); ++i) {
        string szSavePredictFile = szImageNameList[i];
        // predict
        int nObjectId = objectFeature.predict(DEFAULT_DATA_PATH, (char *)szSavePredictFile.c_str());
        cout << "Predict image id " << nObjectId << ", actual id " << nImageIdList[i] << " for " << szSavePredictFile << endl;

        ++nTotle;
        if (nObjectId != nImageIdList[i]){
            ++nFail;
        }
    }

    cout << "验证结束，总计：" << nTotle << "， 失败：" << nFail << ", 成功率" << (nTotle-nFail)*100/nTotle << "%"<< endl;
    return true;
}

bool extractFeatures(CObjectFeature &objectFeature)
{
    // add image
    vector<int> nImageIdList;
    vector<string> szImageNameList;
    vector<string> szImageUrlList;
    vector<string> szImageRawList;

    LoadImages(DEFAULT_DATA_FILE_ANNO, nImageIdList, szImageNameList, szImageRawList, szImageUrlList);

    for (int i = 0; i < nImageIdList.size(); ++i) {
        objectFeature.add_image((char *)szImageNameList[i].c_str(), nImageIdList[i]);
    }

    cout << "add images: " << nImageIdList.size() << endl;

    // extract feathures
    objectFeature.extract_features(DEFAULT_DATA_PATH);
    return true;
}

bool test_train()
{
    CObjectFeature objectFeature;
    extractFeatures(objectFeature);

    // training images
    objectFeature.train(DEFAULT_DATA_PATH);

    // verify samples
    verifySamples(objectFeature);
    return true;
}

bool testPredict(CObjectFeature &objectFeature)
{
    // loop all image files from the path
    _finddata_t fileDir;
    string dir = DEFAULT_IMAGE_PATH;
    string searchDir = dir + "/*.jpg";
    long lfDir;

    if ((lfDir = _findfirst(searchDir.c_str(), &fileDir)) == -1l)
        cout << "No file is found at " << dir << endl;
    else{
        cout << "file list:\n";
        do{
            //cout << fileDir.name << endl;
            string szSavePredictFile = dir + "/" + fileDir.name;

            // predict
            int nObjectId = objectFeature.predict(DEFAULT_DATA_PATH, (char *)szSavePredictFile.c_str());
            cout << "Predict image id " << nObjectId << " for " << szSavePredictFile << endl;
        } while (_findnext(lfDir, &fileDir) == 0);
    }

    _findclose(lfDir);
    return true;
}

bool test_surf()
{
    CObjectFeature objectFeature;
    // extractFeatures(objectFeature);
    
    // test_sample();

    // test new images
    testPredict(objectFeature);
    return true;
}

bool getUsbFile(_TCHAR* szUsbPath, string &szFile)
{
    // loop all image files from the path
    _finddata_t fileDir;
    string dir = szUsbPath;
    string searchDir = dir + "/*.jpg";
    long lfDir;

    if ((lfDir = _findfirst(searchDir.c_str(), &fileDir)) == -1l){
        // cout << "No file is found at " << dir << endl;
        _findclose(lfDir);
        return false;
    }
    else{
        //cout << "file list:\n";
        do {
            //cout << fileDir.name << endl;
            string fileName = fileDir.name;
            szFile = dir + "/" + fileDir.name;

            // cout << "find a file: " << szFile << endl;
            CUtility::debug("find a file: %s", szFile.c_str());
            break;
        } while (_findnext(lfDir, &fileDir) == 0);
    }

    _findclose(lfDir);
    return true;
}


void dumpFile(int nObjectId, string szPredictFile, string szResult, string szUrl, string szOutPathFile)
{
    // string szPath, szName;;
    //CUtility::getPathFile(szResult, szPath, szName);
    //string szLocalPath = CUtility::getCwd() + DEFAULT_DATA_RAW_PATH + szName;

    string szOutPath, szName;
    CUtility::getPathFile(szPredictFile, szOutPath, szName);

    ofstream sfile(szOutPathFile, ios::out);
    assert(sfile.is_open());

    time_t now_time;
    now_time = time(NULL);

    sfile << "{\"ts\":" << now_time << ", \"predict\":\"" << szName << "\", \"imageid\":" << nObjectId << ", \"url\":\"" << szUrl << "\"}" << endl;
    sfile.close();
    cout << "dump reslult to file：" << szOutPathFile << endl;
}

void initDumpFile(string szOutPathFile)
{
    ofstream sfile(szOutPathFile, ios::out);
    assert(sfile.is_open());

    sfile << "{\"ts\":" << 0 << ", \"predict\":\"" << "" <<"\", \"imageid\":" << -1 << ", \"url\":\"" << "" << "\"}" << endl;
    sfile.close();
    cout << "init dump reslult to file：" << szOutPathFile << endl;
}

bool showFile(string szFile)
{
    //读入图像
    Mat img = imread(szFile, CV_LOAD_IMAGE_COLOR);
    if (img.empty()) {
        cout << "Could not open or find the image: " << szFile << endl;
        return false;
    }

    //创建窗口
    namedWindow("预览", CV_WINDOW_AUTOSIZE);

    //显示图像
    imshow("预览", CUtility::normalSize(img));
    return true;
}

bool savePredict(string szFile, string szOutPathFile, string &szSavedPathFile)
{
    string szOutPath, szName;
    CUtility::getPathFile(szOutPathFile, szOutPath, szName);

    string szPredictPath, szPredictName;
    CUtility::getPathFile(szFile, szPredictPath, szPredictName);
    szSavedPathFile = szOutPath + "/" + szPredictName;

    // save the predict image file
    ::CopyFile(szFile.c_str(), szSavedPathFile.c_str(), FALSE);
    return true;
}

bool usbPredict(CObjectFeature &objectFeature, string szFile, vector<string> &szImageNameList, vector<string> &szImageUrlList, string szOutPathFile)
{
    string szSavedPathFile;
    savePredict(szFile, szOutPathFile, szSavedPathFile);

    // predict
    int nObjectId = objectFeature.predict(DEFAULT_DATA_PATH, (char *)szFile.c_str());
    if (nObjectId <= 0 || nObjectId > g_nMaxObjectID){
        cout << "fail to predict：" << nObjectId << " (max " << g_nMaxObjectID << ")" << endl;
        dumpFile(-1, "", "", "", szOutPathFile); // dump error to file
        return false;
    }

    //assert(nObjectId <= g_nMaxBowID);
    string szResultFile = szImageNameList[nObjectId - 1];
    string szPredictUrl = szImageUrlList[nObjectId - 1];
    cout << "输入文件: " << szFile << ", 匹配: " << szResultFile << ", 图像id:" << nObjectId << endl;
    cout << "url:" << szPredictUrl << endl;
    // CUtility::debug("predict: %d - %s for file: %s", nObjectId, szPredictFile.c_str(), szFile.c_str());

    // write result to file
    dumpFile(nObjectId, szSavedPathFile, szResultFile, szPredictUrl, szOutPathFile);

    // open url
    // ShellExecute(NULL, _T("open"), szPredictUrl.c_str(), NULL, NULL, SW_SHOW);

    // return showFile(szPredictFile);
    return true;
}

void test_usb(_TCHAR* szUsbPath, _TCHAR* szMillSeconds, _TCHAR* szOutPathFile)
{
    // check input args
    int nMillSeconds = atoi(szMillSeconds);
    if (nMillSeconds < 100 || nMillSeconds > 10000) {
        nMillSeconds = 1000;
    }

    string szPull = "adb pull ";
    szPull = szPull + szUsbPath + " >> adbpull.txt";

    string szRm = "adb shell \"rm ";
    szRm += szUsbPath;
    szRm += "/*.jpg\" >> adbrm.txt";

    cout << "excute adb commands:" << endl;
    cout << szPull << endl;
    cout << szRm << endl;
    cout << "delay millseconds: " << nMillSeconds << endl;

    // init dump file
    initDumpFile(szOutPathFile);

    // add image
    vector<int> nImageIdList;
    vector<string> szImageNameList;
    vector<string> szImageUrlList;
    vector<string> szImageRawList;

    LoadImages(DEFAULT_DATA_FILE_ANNO, nImageIdList, szImageNameList, szImageRawList, szImageUrlList);

    // load a pic from usb
    cout << "请用USB连接的手机拍照,按Q键退出..." << endl;
    CObjectFeature objectFeature;
    int nFail = 0;
    char key_press = 0;
    while (1){
        if (_kbhit()) // 如果有键盘输入
        {
            key_press = _getch(); // (char)waitKey(500);
            cout << "key: " << key_press << endl;
            if (key_press == 'q') {
                cout << "用户已退出程序" << endl;
                break;
            }
        }

        // get image from usb
        //string szCmd = szPull + " && " + szRm;
        //cout << szCmd << endl;
        //int nRet = WinExec(szPull.c_str(), SW_HIDE);
        //nRet = WinExec(szRm.c_str(), SW_HIDE);
        int nRet = system(szPull.c_str());
        // cout << endl << szPull << ":" << nRet << endl;
        if (nRet){
            cout << "请使用USB线连接手机并安装相应手机驱动！" << endl;
            system("pause");
        }

        nRet = system(szRm.c_str());

        string szFile;
        if (getUsbFile(".", szFile)){
            // test new images
            if (usbPredict(objectFeature, szFile, szImageNameList, szImageUrlList, szOutPathFile)){
                // remove it
                remove(szFile.c_str());
            }
            else{
                ++nFail;
                cout << "retry " << nFail << " times" << endl;
                if (nFail >= 2){
                    // remove it
                    remove(szFile.c_str());
                    nFail = 0;
                }
            }
        }

        ::Sleep(nMillSeconds);
    }

    // init dump file
    initDumpFile(szOutPathFile);
}

/////////////////////////////////////////////////////////////////////
void verifyHistogram(CObjectFeature &objectFeature)
{
    // add image
    vector<int> nImageIdList;
    vector<string> szImageNameList;
    vector<string> szImageUrlList;
    vector<string> szImageRawList;
    LoadImages(DEFAULT_DATA_FILE_ANNO, nImageIdList, szImageNameList, szImageRawList, szImageUrlList);

    map<int, double> mapDist;
    int nFail = 0, nTotle = 0;
    for (int i = 0; i < szImageNameList.size(); ++i) {
        string szSavePredictFile = szImageNameList[i];

        // predict
        int  nIndex = objectFeature.computHistogram(DEFAULT_DATA_PATH, (char *)szSavePredictFile.c_str(), mapDist);
        cout << "Predict image id " << nImageIdList[nIndex] << ", actual id " << nImageIdList[i] << " for " << szSavePredictFile << endl;

        objectFeature.checkColors(szSavePredictFile, szImageNameList[nIndex], 0.3);
        for (int j = 0; j < szImageNameList.size(); ++j){
            // check match
            objectFeature.checkColors(szSavePredictFile, szImageNameList[j], 0.3);
        }

        ++nTotle;
        if (nImageIdList[nIndex] != nImageIdList[i]){
            ++nFail;
        }
    }

    cout << "验证结束，总计：" << nTotle << "， 失败：" << nFail << ", 成功率" << (nTotle - nFail) * 100 / nTotle << "%" << endl;
}

void test_train_histogram()
{
    CObjectFeature objectFeature;
    extractFeatures(objectFeature);

    // verfiy histogram
    verifyHistogram(objectFeature);
}

void test_verify_histogram()
{
    CObjectFeature objectFeature;

    // verfiy histogram
    verifyHistogram(objectFeature);
}

bool test_predict_histogram(CObjectFeature &objectFeature, string szFile, vector<int> &nImageIdList, vector<string> &szImageNameList, vector<string> &szImageRawList)
{
    map<int, double> mapDist;

    // predict
    int  nIndex = objectFeature.computHistogram(DEFAULT_DATA_PATH, (char *)szFile.c_str(), mapDist);
    if (nIndex > 0){
        cout << "输入文件: " << szFile << ", 匹配: " << szImageNameList[nIndex] << ", 图像id:" << nImageIdList[nIndex] << endl;
        showFile(szImageRawList[nIndex]);
    }
    else{
        cout << "输入文件: " << szFile << ", 未匹配" << endl;
    }

    // check match
    //if (!objectFeature.checkColors(szFile, szImageNameList[nIndex], 0.5)){
    //    cout << "输入文件: " << szFile << ", 未匹配" << endl;

    //    //for (int i = 0; i < nImageIdList.size(); ++i){
    //    //    objectFeature.checkColors(szFile, szImageNameList[i], 0.5);
    //    //}

    //    return false;
    //}

    // show match file
    
    return true;
}

void test_usb_histogram(_TCHAR* szUsbPath)
{
    // check input args
    string szPull = "adb pull ";
    szPull = szPull + szUsbPath + " >> adbpull.txt";

    string szRm = "adb shell \"rm ";
    szRm += szUsbPath;
    szRm += "/*.jpg\" >> adbrm.txt";

    cout << "excute adb commands:" << endl;
    cout << szPull << endl;
    cout << szRm << endl;

    // add image
    vector<int> nImageIdList;
    vector<string> szImageNameList;
    vector<string> szImageUrlList;
    vector<string> szImageRawList;

    LoadImages(DEFAULT_DATA_FILE_ANNO, nImageIdList, szImageNameList, szImageRawList, szImageUrlList);

    // load a pic from usb
    cout << "请用USB连接的手机拍照,按Q键退出..." << endl;
    CObjectFeature objectFeature;
    int nFail = 0;
    char key_press = 0;
    while (1){
        //if (_kbhit()) // 如果有键盘输入
        //{
        //    key_press = _getch(); // (char)waitKey(500);
        //    cout << "key: " << key_press << endl;
        //    if (key_press == 'q') {
        //        cout << "用户已退出程序" << endl;
        //        break;
        //    }
        //}

        key_press = (char)waitKey(100);
        if (key_press == 'q') {
            cout << "用户已退出程序" << endl;
            break;
        }

        // get image from usb
        int nRet = system(szPull.c_str());
        // cout << endl << szPull << ":" << nRet << endl;
        if (nRet){
            cout << "请使用USB线连接手机并安装相应手机驱动！" << endl;
            system("pause");
        }

        nRet = system(szRm.c_str());

        string szFile;
        if (getUsbFile(".", szFile)){
            // test new images
            if (test_predict_histogram(objectFeature, szFile, nImageIdList, szImageNameList, szImageRawList)){
                // remove it
                remove(szFile.c_str());
            }
            else{
                ++nFail;
                cout << "retry " << nFail << " times" << endl;
                if (nFail >= 2){
                    // remove it
                    remove(szFile.c_str());
                    nFail = 0;
                }
            }
        }

        ::Sleep(1000);
    }
}

////////////////////////////////////////////////////////////////////

void showVersion()
{
    cout << "imageSearch" << endl;
    cout << "version:" << "1.0.0" << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc < 2) {
        cerr << "参数错误" << endl;
        system("pause");
        exit(__LINE__);
    }

    showVersion();
    if (!strcmp(argv[1], "-train")) {
        cout << "当前模式:训练" << endl;
        test_train();
    }
    if (!strcmp(argv[1], "-train_hist")) {
        cout << "当前模式:训练直方图" << endl;
        test_train_histogram();
    }
    if (!strcmp(argv[1], "-verify_hist")) {
        cout << "当前模式:验证直方图" << endl;
        test_verify_histogram();
    }
    else if (!strcmp(argv[1], "-detect")) {
        cout << "当前模式:摄像头采集预测" << endl;
        test_detect();  
    }
    else if (!strcmp(argv[1], "-test")) {
        cout << "当前模式:测试" << endl;
        test_surf(); 
    }
    else if (!strcmp(argv[1], "-usb")) {
        cout << "当前模式:usb采集." << endl;

        if (argc >= 5){
            test_usb(argv[2], argv[3], argv[4]);
        }
        else{
            cerr << "参数错误" << endl;
            cerr << "usage:" << endl;
            cerr << "ImageSearch -usb <adbUsbPath> <delay_millseconds> <ouputPath> ---usb采集 手机相机adb路径 采集延时毫秒数 输出结果文件名" << endl;
            system("pause");
        }
    }
    else if (!strcmp(argv[1], "-usb_hist")) {
        cout << "当前模式:摄像头采集直方图预测" << endl;

        if (argc >= 3){
            test_usb_histogram(argv[2]);
        }
        else{
            cerr << "参数错误" << endl;
            cerr << "usage:" << endl;
            cerr << "ImageSearch -usb <adbUsbPath> ---usb采集 手机相机adb路径" << endl;
            system("pause");
        }
    }
    else {
        cerr << "参数错误" << endl;
        cerr << "usage:" << endl;
        cerr << "ImageSearch -test ---测试" << endl;
        system("pause");
    }

    system("pause");
    return 0;
}