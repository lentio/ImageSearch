#ifndef ObjectFeature_h__
#define ObjectFeature_h__

typedef void(*debug_callback)(char*, int);

class CObjectFeature
{
public:
    CObjectFeature();
    virtual ~CObjectFeature();

    int add_image(char* image_full_path, int object_id);
    void clear_images();
    int extract_features(char* path_to_save);
    int train(char* path_to_save);
    int predict(char* path_to_save, char* image_full_path);
    void set_debug_callback(debug_callback cb);
    void debug(char* format, ...);

    int computHistogram(char* path_to_save, char* image_full_path, map<int, double> &mapDist);

private:
    map<int, vector<string> > m_images_path;
    debug_callback m_fnDebug;

    int nMaxBowClasses;
public:
    bool checkMatch(string szSrc, string szDst, float best);
    bool extractSurf(string szFile, Mat& surfDescriptor);
    double compareSURFDescriptors(const float* d1, const float* d2, double best, int length);
    bool checkColors(string szSrc, string szDst, double threshold);
    bool extractColors(string szFile, Mat& hist);
};

#endif // ObjectFeature_h__
