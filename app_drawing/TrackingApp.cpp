//
// Created by Liza on 5/1/2023.
//

#include "TrackingApp.h"
#include <memory>
#include "../sources/LeapConnection.h"

std::unique_ptr<TrackingApp> __app;

/** Match 3D points with 2d display*/
void frameDisplayCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    if (frame->nHands > 0) {
        for (int i = 0; i < frame->nHands; ++i) {
            auto &hand = frame->pHands[i];

            auto& indexFinger = hand.index.distal.next_joint;
            if (__app->_attachHandToScreen(indexFinger)) {
                __app->move_event = true;
                auto surfaceDistance = __app->_calibration.pointToSurfaceDistance(cv::Point3f(indexFinger.x,
                                                                                      indexFinger.y,
                                                                                      indexFinger.z));
                if (fabs(surfaceDistance) < __app->tap_threshold)
                    __app->press_event = true;
            }
        }
    }
}
TrackingApp::TrackingApp(){
    display_image = cv::Mat(h, w, CV_8UC3, cv::Scalar(255, 255, 255));
    __app = std::unique_ptr<TrackingApp>(this);
}
TrackingApp::~TrackingApp(){
    __app = nullptr;
}

void TrackingApp::run(){
    LeapConnection connection;
    connection.tracking_callback = frameDisplayCallback;

    _calibration = CalibrationArea("calib.txt");
    _calibration.calibrate();

    connection.open();
    _trackingHandCursor();

    connection.close();
}

/** Main function for display cursor depending on hand pose*/
void TrackingApp::_trackingHandCursor(){
    w = _calibration.aspectRatio() * 500.0;
    h = 500.0;
    cv::namedWindow("Display window", cv::WINDOW_FREERATIO);

    while (cv::waitKey(delay) != 27) {
        display_image = cv::Mat(h, w, CV_8UC3, cv::Scalar(255, 255, 255));

        if (move_event) {
            circle(display_image, cv::Point(x, y), 20, cv::Scalar(0, 0, 255), 2);
            move_event = false;
        }
        if (press_event) {
            circle(display_image, cv::Point(x, y), 40, cv::Scalar(0, 255, 0), 5);
            press_event = false;
        }

        cv::imshow("Display window", display_image);
    }
    cv::destroyAllWindows();
}

bool TrackingApp::_attachHandToScreen(LEAP_VECTOR& coords3D){
    // x - left, y - down, z - on me
    cv::Point3f point = {coords3D.x, coords3D.y, coords3D.z};

    // check if coord inside interaction area
    if (!_calibration.isPointInside(point))
        return false;

    cv::Point2f uv = _calibration.findPointUV(point);
    x = uv.x * w;
    y = uv.y * h;

    return true;
}