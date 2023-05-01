//
// Created by Liza on 5/1/2023.
//

#ifndef LEAPC_TEST_TRACKINGAPP_H
#define LEAPC_TEST_TRACKINGAPP_H
#include "CalibData.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <memory>

class TrackingApp {
    public:
        TrackingApp();
        ~TrackingApp();
        void run();

    public:
        float delay = 10;
        float tap_threshold = 10;

        std::atomic_int x, y = 0;
        std::atomic_int last_h = 0;
        std::atomic_int w, h;
        cv::Mat display_image;

        std::atomic_bool press_event = false;
        std::atomic_bool move_event = false;
        CalibData _calibration;

    public:
        bool _attachHandToScreen(LEAP_VECTOR& coords3D);
    private:
        void _trackingHandCursor();

};


#endif //LEAPC_TEST_TRACKINGAPP_H
