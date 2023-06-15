//
// Created by Liza on 6/9/2023.
//

#ifndef LEAPC_TEST_GESTURERECOGNIZER_H
#define LEAPC_TEST_GESTURERECOGNIZER_H
#include <opencv2/highgui.hpp>
#include "CalibrationArea.h"
#include "LeapC.h"
#include <list>
#include <thread>
#include <iostream>

enum EGestureType{ eLeftDown = 0, eLeftUp, eRightUp, eRightDown, eMove, eDoubleClick, eNone};

class FingersContainer{
    public:
        struct FingersData{
            cv::Point3f index;
            cv::Point3f middle;
            double interdigital_dist;
            int64_t time;
        };
    public:
        void clear();
        FingersData& back();
        FingersData& back(int i);
        int size() const {return  data.size();}
        FingersData& add(cv::Point3f& indexCoords,
                         cv::Point3f& middleCoords,
                         double interdigital_dist,
                         int64_t timestamp);

    private:
        int max_count = 10;
        int64_t initial_time = 0;
        std::mutex mutex;
        std::list<FingersData> data;
};

class GestureRecognizer {
    public:
        GestureRecognizer(int screen_w, int screen_h,
                          const std::string calib_fname = "calib.txt")
                          : screenW(screen_w),
                          screenH(screen_h),
                          _calibration(calib_fname){_calibration.calibrate();}
        std::pair<EGestureType, cv::Point2f> recognize(cv::Point3f& indexCoords,
                                                       cv::Point3f& middleCoords,
                                                       int64_t timestamp);
        cv::Point3f mapFingerPosition(cv::Point3f& coords3D);
    public:
        float interaction_threshold = 100;
        float press_threshold = 10;
        float interdigital_threshold = 20;
        float max_doubleclick_delay = 500; // ms
        int screenW;
        int screenH;

    private:
        CalibrationArea _calibration;
        FingersContainer _fingersContainer;

        std::atomic_bool press_event = false;
        std::atomic_bool right_press_event = false;
        std::atomic_int64_t last_release_time = -1000000;
};


#endif //LEAPC_TEST_GESTURERECOGNIZER_H
