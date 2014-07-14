#pragma once

class CUtility
{
public:
    CUtility();
    ~CUtility();

    static void debug(char* format, ...);
    static Mat normalSize(Mat img);

    static bool getPathFile(string szPathFile, string &szFilePath, string &szFileName);

    static string getCwd();

    static string showTime();
};

