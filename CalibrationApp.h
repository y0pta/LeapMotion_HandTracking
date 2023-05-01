//
// Created by Liza on 5/1/2023.
//

#ifndef LEAPC_TEST_CALIBRATIONAPP_H
#define LEAPC_TEST_CALIBRATIONAPP_H
#include "CalibData.h"
#include <atomic>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "LeapConnection.h"

class CalibrationApp {
    public:
        CalibrationApp();
        ~CalibrationApp();
        void run();

    public:
        std::vector<LEAP_VECTOR> calibrationPoints;
        std::atomic_bool isCalibrating = false;
        std::atomic_bool recordEvent = false;
        std::atomic_int last_key = 0;
        float delay = 10;
        std::mutex image_mutex;
        cv::Mat curImage;

    private:
        bool _getCalibrationPoints();

    private:
        CalibData _data;
};


#endif //LEAPC_TEST_CALIBRATIONAPP_H
