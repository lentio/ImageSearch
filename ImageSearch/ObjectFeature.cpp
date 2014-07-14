#include "stdafx.h"
#include "ObjectFeature.h"
#include "Utility.h"

CObjectFeature::CObjectFeature()
{
    initModule_nonfree();
    nMaxBowClasses = OF_BOW_MAX_CLASSES;
}


CObjectFeature::~CObjectFeature()
{
}

int CObjectFeature::add_image(char* image_full_path, int object_id)
{
    debug("添加图像[%s]\n", image_full_path);
    m_images_path[object_id].push_back(image_full_path);
    //nMaxBowClasses = m_images_path.size();
    return 0;
}

void CObjectFeature::clear_images()
{
    debug("内存中的图像路径已全部清空");
    m_images_path.clear();
}

int CObjectFeature::extract_features(char* path_to_save)
{
    vector<Mat> lstDescriptors;
    //map<int, vector<Mat>> mapDescriptors;
    
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
    Ptr<DescriptorExtractor> extractor = new SurfDescriptorExtractor();
    BOWImgDescriptorExtractor bow_extractor(extractor, matcher);
    SurfFeatureDetector detector(500);

    TermCriteria terminate_criterion;
    terminate_criterion.epsilon = FLT_EPSILON;
    BOWKMeansTrainer bow_trainer(nMaxBowClasses, terminate_criterion); // nMaxBowClasses/*聚类数量*/

    // 获取特征并添加聚类分类器
	// vs2013
    //for (auto& obj : m_images_path) {
    //    vector<string> images_path = obj.second;
    //    for (auto& image_path : images_path) {
    //        Mat image = imread(image_path, CV_LOAD_IMAGE_GRAYSCALE);
    //        vector<KeyPoint> keypoint;
    //        detector.detect(image, keypoint);
    //        Mat descriptor;
    //        extractor->compute(image, keypoint, descriptor);
    //        if (!descriptor.empty()) {
    //            bow_trainer.add(descriptor);
    //        }
    //        debug("stage1: 文件[%s]处理完成", image_path);
    //    }
    //}
	// vs2010
	for (auto obj = m_images_path.begin(); obj != m_images_path.end(); ++obj) {
		vector<string> images_path = obj->second;
		for (auto image_path = images_path.begin(); image_path != images_path.end(); ++image_path) {
            debug("stage 1: extract surf features 文件[%s]", (*image_path).c_str());
			Mat image = imread(*image_path, CV_LOAD_IMAGE_GRAYSCALE);
            assert(image.rows != 0);

            // prepare and resize
            // scale into 800 x ...
            //Size dsize = Size(IS_IMAGE_NORMAL_SIZE, (image.cols * IS_IMAGE_NORMAL_SIZE / image.rows));
            //Mat scaleImage = Mat(dsize, CV_32S);
            //resize(image, scaleImage, dsize);
            Mat scaleImage = CUtility::normalSize(image);
            //equalizeHist(scaleImage, scaleImage);

			vector<KeyPoint> keypoint;
            detector.detect(scaleImage, keypoint);
			Mat descriptor;
            extractor->compute(scaleImage, keypoint, descriptor);
			if (!descriptor.empty()) {
				bow_trainer.add(descriptor);
			}
		}
	}

    // 通过聚类创建visual word
    debug("开始聚类: %d", nMaxBowClasses); // nMaxBowClasses
    Mat vocabularys = bow_trainer.cluster();
    bow_extractor.setVocabulary(vocabularys);
    debug("聚类完成");

	// vs2013
    //for (auto& obj : m_images_path) {
    //    vector<string> images_path = obj.second;
    //    for (auto& image_path : images_path) {
    //        Mat image = imread(image_path, CV_LOAD_IMAGE_GRAYSCALE);
    //        vector<KeyPoint> keypoint;
    //        detector.detect(image, keypoint);
    //        Mat descriptor;
    //        bow_extractor.compute(image, keypoint, descriptor);
    //        if (!descriptor.empty()) {
    //            lstDescriptors.push_back(descriptor);
    //        }
    //        debug("stage2: 文件[%s]处理完成", image_path);
    //    }
    //}
	// vs2010
	for (auto obj = m_images_path.begin(); obj != m_images_path.end(); ++obj) {
		vector<string> images_path = obj->second;
        int nObjectId = obj->first;
		for (auto image_path = images_path.begin(); image_path != images_path.end(); ++image_path) {
            debug("stage 2: extract bow descriptor 文件[%s]", (*image_path).c_str());
			Mat image = imread(*image_path, CV_LOAD_IMAGE_GRAYSCALE);

            // prepare and resize
            // scale into 800 x ...
            //Size dsize = Size(IS_IMAGE_NORMAL_SIZE, (image.cols * IS_IMAGE_NORMAL_SIZE / image.rows));
            //Mat scaleImage = Mat(dsize, CV_32S);
            //resize(image, scaleImage, dsize);
            Mat scaleImage = CUtility::normalSize(image);
			//equalizeHist(image, image);

			vector<KeyPoint> keypoint;
            detector.detect(scaleImage, keypoint);
			Mat descriptor;
            bow_extractor.compute(scaleImage, keypoint, descriptor);
			if (!descriptor.empty()) {
				lstDescriptors.push_back(descriptor);
                //mapDescriptors[nObjectId].push_back(descriptor);
			}
		}
	}

    FileStorage fs(string(path_to_save)+"\\features.xml", FileStorage::WRITE);
    if (fs.isOpened()) {
        fs << "descriptors" << lstDescriptors;
        fs << "vocabularys" << vocabularys;
        //fs << "histogram" << mapDescriptors;
    }
    fs.release();
    debug("保存特征完成");

    return 0;
}

int CObjectFeature::train(char* path_to_save)
{
    vector<Mat> lstDescriptors;
    Mat vocabularys;

    FileStorage fs(string(path_to_save) + "\\features.xml", FileStorage::READ);
    if (fs.isOpened()) {
        fs["descriptors"] >> lstDescriptors;
        fs["vocabularys"] >> vocabularys;
    }
    fs.release(); 

    int num_of_images = 0;
	// vs2013
    //for (auto& obj : m_images_path) {
    //    num_of_images += obj.second.size();
    //}
	// vs2010
	for (auto obj = m_images_path.begin(); obj != m_images_path.end(); ++obj) {
		num_of_images += obj->second.size();
	}

    Mat train_data(num_of_images, vocabularys.rows, CV_32FC1);
    Mat responses(num_of_images, 1, CV_32SC1);
    // 填充train_data结构
    for (int i = 0; i < num_of_images; ++i) {
        Mat sub_mat = train_data.row(i);
        lstDescriptors[i].copyTo(sub_mat);
    }
    // 填充responses结构
	// vs2013
    //for (int i = 0; i < num_of_images;) {
    //    for (auto& obj : m_images_path) {
    //        int object_id = obj.first;
    //        vector<string> images_path = obj.second;
    //        for (auto& _ : images_path) {
    //            responses.at<int>(i) = object_id;
    //            ++i;
    //        }
    //    }
    //}
	// vs2010
	for (int i = 0; i < num_of_images;) {
		for (auto obj = m_images_path.begin(); obj != m_images_path.end(); ++obj) {
			int object_id = obj->first;
			vector<string> images_path = obj->second;
			for (auto it = images_path.begin(); it != images_path.end(); ++it) {
				responses.at<int>(i) = object_id;
				++i;
			}
		}
	}

    debug("training with SVM");

    // 训练
    //CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
    //CvSVMParams svm_param = CvSVMParams(CvSVM::C_SVC, CvSVM::RBF, 10.0, 8.0, 1.0, 10.0, 0.5, 0.1, NULL, criteria);
	//SVM svm;
	//svm.train(train_data, responses, cv::Mat(), cv::Mat(), svm_param);
	//svm.save((string(path_to_save) + "\\svm-trained.xml").c_str());
	CvSVMParams svmParams;
	svmParams.svm_type = CvSVM::C_SVC;
	svmParams.kernel_type = CvSVM::RBF;
	CvParamGrid c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid;
	c_grid = CvSVM::get_default_grid(CvSVM::GAMMA);
	p_grid = CvSVM::get_default_grid(CvSVM::P);
	p_grid.step = 0;
	nu_grid = CvSVM::get_default_grid(CvSVM::NU);
	nu_grid.step = 0;
	coef_grid = CvSVM::get_default_grid(CvSVM::COEF);
	coef_grid.step = 0;
	degree_grid = CvSVM::get_default_grid(CvSVM::DEGREE);
	degree_grid.step = 0;
	SVM svm;
    svm.train_auto(train_data, responses, Mat(), Mat(), svmParams, OF_BOW_MAX_Fold, c_grid, gamma_grid, p_grid, nu_grid, coef_grid, degree_grid);
    string svmSaver = (string(path_to_save) + "\\svm-trained.xml").c_str();
    svm.save((string(path_to_save) + "\\svm-trained.xml").c_str());
    
    debug("save svm into %s", svmSaver.c_str());
    return 0;
}

int CObjectFeature::predict(char* path_to_save, char* image_full_path)
{
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
    Ptr<DescriptorExtractor> extractor = new SurfDescriptorExtractor();
    BOWImgDescriptorExtractor bow_extractor(extractor, matcher);
    SurfFeatureDetector detector(500);

    Mat vocabularys;
    FileStorage fs(string(path_to_save) + "\\features.xml", FileStorage::READ);
    if (fs.isOpened()) {
        fs["vocabularys"] >> vocabularys;
    }
    fs.release();

    Mat descriptor;
    bow_extractor.setVocabulary(vocabularys);
	Mat image = imread(image_full_path, CV_LOAD_IMAGE_GRAYSCALE);
    if (image.rows <= 0){
        //|| (image.rows < IS_IMAGE_MIN_ROWS && image.cols < IS_IMAGE_MIN_COLS)
        //|| (image.rows < IS_IMAGE_MIN_COLS && image.cols < IS_IMAGE_MIN_ROWS)){
        debug("!!!fail to read image: %s with error size: %d x %d", image_full_path, image.rows, image.cols);
        return -1;
    }

    debug("load image: %s size: %d x %d\n", image_full_path, image.rows, image.cols);

    //assert(image.rows > 0);
	//equalizeHist(image, image);
	//Mat image;

    // prepare and resize
    // scale into 800 x ...
    //Size dsize = Size(IS_IMAGE_NORMAL_SIZE, (image.cols * IS_IMAGE_NORMAL_SIZE / image.rows));
    //Mat scaleImage = Mat(dsize, CV_32S);
    //resize(image, scaleImage, dsize);
    Mat scaleImage = CUtility::normalSize(image);
    //equalizeHist(scaleImage, scaleImage);

    vector<KeyPoint> keypoint;
    detector.detect(scaleImage, keypoint);
    bow_extractor.compute(scaleImage, keypoint, descriptor);
    
    SVM svm;
    svm.load((string(path_to_save) + "\\svm-trained.xml").c_str());

    float score = 0.0f;
    try {
        score = svm.predict(descriptor, true);
    } catch (exception& e) {
        debug("SVM异常:%s", e.what());
    }

    return (int)(score+0.5);
}

void CObjectFeature::set_debug_callback(debug_callback cb)
{
    m_fnDebug = cb;
}

void CObjectFeature::debug(char* format, ...)
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
    if (m_fnDebug) {
        (*m_fnDebug)(buf, strlen(buf));
    }
}

/*
void CObjectFeature::resize(Mat &src, Mat &dst, Size dsize)
{
    // scale into 90x100
    Size dsize = Size(7, 4);
    scaleMouth = Mat(dsize, CV_32S);
    resize(cutFace, scaleMouth, dsize);

}
*/

///////////////////////////////////////////////////////////////////////////
int CObjectFeature::computHistogram(char* path_to_save, char* image_full_path, map<int, double> &mapDist)
{
    Mat vocabularys;
    vector<Mat> lstDescriptors;
    map<int, vector<Mat>> mapDescriptors;

    FileStorage fs(string(path_to_save) + "\\features.xml", FileStorage::READ);
    if (fs.isOpened()) {
        fs["vocabularys"] >> vocabularys;
        fs["descriptors"] >> lstDescriptors;
        //fs ["histogram"] >> mapDescriptors;
    }
    fs.release();

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
    Ptr<DescriptorExtractor> extractor = new SurfDescriptorExtractor();
    BOWImgDescriptorExtractor bow_extractor(extractor, matcher);
    SurfFeatureDetector detector(500);

    bow_extractor.setVocabulary(vocabularys);
    Mat image = imread(image_full_path, CV_LOAD_IMAGE_GRAYSCALE);
    if (image.rows <= 0){
        debug("!!!fail to read image: %s with error size: %d x %d", image_full_path, image.rows, image.cols);
        return -1;
    }

    debug("load image: %s size: %d x %d\n", image_full_path, image.rows, image.cols);

    Mat scaleImage = CUtility::normalSize(image);
    //equalizeHist(scaleImage, scaleImage);

    Mat descriptor;
    vector<KeyPoint> keypoint;
    detector.detect(scaleImage, keypoint);
    bow_extractor.compute(scaleImage, keypoint, descriptor);

    // compare histogram one by one
    double fMax = 0.0;
    int nIndex = 0;

    for (int i = 0; i < lstDescriptors.size(); ++i){
        double dist = compareHist(lstDescriptors[i], descriptor, CV_COMP_INTERSECT); // CV_COMP_BHATTACHARYYA); // CV_COMP_INTERSECT);
        mapDist.insert(pair<int, double>(i, dist));
        if (dist > fMax){
            fMax = dist;
            nIndex = i;
        }

        debug("loop %d dist %lf\n", i, dist);
    }

    debug("best dist %lf\n", fMax);
    if (fMax < 0.6){
        return -1;
    }

    return nIndex;
}

bool CObjectFeature::checkMatch(string szSrc, string szDst, float best)
{
    Mat srcDescriptor, dstDescriptor;
    if (!extractSurf(szSrc, srcDescriptor) || !extractSurf(szDst, dstDescriptor)){

        return false;
    }

    //double d, dist1 = 1e6, dist2 = 1e6;
    //int length = (int)(srcDescriptor.rows*srcDescriptor.cols);

    //d = compareSURFDescriptors(srcDescriptor.ptr<float>(0), dstDescriptor.ptr<float>(0), dist2, length);

    //vector<DMatch> matches;
    //Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
    //matcher->match(srcDescriptor, dstDescriptor, matches);

    //debug("%s match %s with: %d", szSrc.c_str(), szDst.c_str(), matches.size());

    return true;
}

bool CObjectFeature::extractSurf(string szFile, Mat& surfDescriptor)
{
    Mat image = imread(szFile, CV_LOAD_IMAGE_GRAYSCALE);
    if (image.rows <= 0){
        debug("fail to read image: %s with error size: %d x %d", szFile, image.rows, image.cols);
        return false;
    }

    debug("load image: %s size: %d x %d\n", szFile.c_str(), image.rows, image.cols);

    Mat scaleImage = CUtility::normalSize(image);
    //equalizeHist(scaleImage, scaleImage);

    // Mat descriptor;
    vector<KeyPoint> keypoint;

    // Detect key points from image
    SurfFeatureDetector detector(500);
    detector.detect(scaleImage, keypoint);

    // Extract descriptors
    Ptr<DescriptorExtractor> extractor = new SurfDescriptorExtractor(); // 提取SURF描述向量
    extractor->compute(scaleImage, keypoint, surfDescriptor);
    return true;
}

double CObjectFeature::compareSURFDescriptors(const float* d1, const float* d2, double best, int length)
{
    double total_cost = 0;
    assert(length % 4 == 0);
    for (int i = 0; i < length; i += 4)
    {
        double t0 = d1[i] - d2[i];
        double t1 = d1[i + 1] - d2[i + 1];
        double t2 = d1[i + 2] - d2[i + 2];
        double t3 = d1[i + 3] - d2[i + 3];
        total_cost += t0*t0 + t1*t1 + t2*t2 + t3*t3;
        if (total_cost > best)
            break;
    }

    return total_cost;
}

bool CObjectFeature::checkColors(string szSrc, string szDst, double threshold)
{
    Mat srcDescriptor, dstDescriptor;
    if (!extractColors(szSrc, srcDescriptor) || !extractColors(szDst, dstDescriptor)){
        return false;
    }

    double dist = compareHist(srcDescriptor, dstDescriptor, CV_COMP_BHATTACHARYYA); // CV_COMP_INTERSECT);
    debug("checkColors %s vs %s with %lf\n", szSrc.c_str(), szDst.c_str(), dist);

    if (dist > threshold){
        return false;
    }

    return true;
}


bool CObjectFeature::extractColors(string szFile, Mat& histImage)
{
    Mat image;
    image = imread(szFile, 1);
    if (image.rows <= 0){
        debug("fail to read image: %s with error size: %d x %d", szFile, image.rows, image.cols);
        return false;
    }

    debug("load image: %s size: %d x %d\n", szFile.c_str(), image.rows, image.cols);

    Mat scaleImage = CUtility::normalSize(image);
    //equalizeHist(scaleImage, scaleImage);

    /// 转换到 HSV
    Mat hsvImage;
    cvtColor(scaleImage, hsvImage, CV_BGR2HSV);

    /// 对hue通道使用30个bin,对saturatoin通道使用32个bin
    int h_bins = 50; int s_bins = 60;
    int histSize[] = { h_bins, s_bins };

    // hue的取值范围从0到256, saturation取值范围从0到180
    float h_ranges[] = { 0, 256 };
    float s_ranges[] = { 0, 180 };

    const float* ranges[] = { h_ranges, s_ranges };

    // 使用第0和第1通道
    int channels[] = { 0, 1 };

    /// 计算HSV图像的直方图
    calcHist(&hsvImage, 1, channels, Mat(), histImage, 2, histSize, ranges, true, false);
    normalize(histImage, histImage, 0, 1, NORM_MINMAX, -1, Mat());
    return true;
}
