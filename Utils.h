//
// Created by Liza on 4/27/2023.
//

#ifndef LEAPC_TEST_UTILS_H
#define LEAPC_TEST_UTILS_H
#include <iostream>
#include "LeapC.h"
#include <opencv2/opencv.hpp>

namespace LeapStatus {
    std::string toString(eLeapRS code);
}

namespace LeapDeviceStatus {
    std::string toString(int deviceStatus);
}

namespace OpencvUtils {
    cv::Mat voidPtrToMat(void *data, int width, int height, int channels);
    void imgConcat(cv::Mat& img1, cv::Mat& img2, cv::Mat& combined);
    void printTextOnImage(cv::Mat& img, const std::string& text);
}

namespace LeapVectorSerialization{
    std::vector<LEAP_VECTOR> readCalibPoints(const std::string& filename);
    void writeCalibPoints(const std::vector<LEAP_VECTOR>& points, const std::string& filename);
}


#endif //LEAPC_TEST_UTILS_H
