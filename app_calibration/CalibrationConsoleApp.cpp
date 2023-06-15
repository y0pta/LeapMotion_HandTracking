//
// Created by Liza on 6/2/2023.
//

#include "CalibrationConsoleApp.h"
#include <iostream>
#include <numeric>
#include <thread>
#include <chrono>
#include "../sources/Utils.h"
std::unique_ptr<CalibrationConsoleApp> __app;

/** Print out position of tail of index finger */
void frameCalibCallbackDebug(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    if (frame->nHands > 0) {
        auto &hand = frame->pHands[0];
        auto &coords = hand.index.distal.next_joint;

        std::cout << coords.x << " "
                  << coords.y << " "
                  << coords.z << std::endl;
    }
}
/** Capture and save position of tail of index finger if both is_calibrating and calib_event are set.*/
void frameCalibConsoleCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    __app->handDetected = frame->nHands > 0;
    if (__app && __app->recordEvent && (frame->nHands > 0)){
        auto& hand = frame->pHands[0];
        auto& coords = hand.index.distal.next_joint;

        __app->calibrationPoints.push_back(coords);
    }
}

CalibrationConsoleApp::CalibrationConsoleApp(){
    __app = std::unique_ptr<CalibrationConsoleApp>(this);
}

CalibrationConsoleApp::~CalibrationConsoleApp(){
    //__app.release();
}

void CalibrationConsoleApp::runDebug()
{
    _connection.tracking_callback = frameCalibCallbackDebug;
    _connection.open();
    AppUtils::waitForKeyPress("Press esc to quit.");
}

void CalibrationConsoleApp::run()
{
    _connection.tracking_callback = frameCalibConsoleCallback;
    _connection.open();

    std::cout << "=== Hand Tracking CalibrationArea ===" << std::endl;
    std::array<cv::Point3f, 4> points;
    points[CalibrationArea::eTopLeft] = _getCalibrationPoint(CalibrationArea::eTopLeft);
    points[CalibrationArea::eBottomRight] = _getCalibrationPoint(CalibrationArea::eBottomRight);
    points[CalibrationArea::eTopRight] = _getCalibrationPoint(CalibrationArea::eTopRight);
    CalibrationArea calib(points);
    calib.calibrate();
    calib.save("calib.txt");
    std::cout << "CalibrationArea done" << std::endl;
}

cv::Point3f CalibrationConsoleApp::_getCalibrationPoint(CalibrationArea::EPointType point)
{
    while(true) {
        AppUtils::waitForKeyPress("Position your index finger to the " + _pointTypeName[point] + " corner of the screen, then press 'R'");
        if (!handDetected){
            std::cout << "Hand is not detected, try again." << std::endl;
        }
        else {
            recordEvent = true;
            break;
        }
    }
    AppUtils::showTimer(3);
    recordEvent = false;

    // record point
    LEAP_VECTOR sum = std::accumulate(calibrationPoints.begin(), calibrationPoints.end(), LEAP_VECTOR{0.0f, 0.0f, 0.0f},
                                      [](const LEAP_VECTOR& lhs, const LEAP_VECTOR& rhs) {
                                          return LEAP_VECTOR{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
                                      });
    cv::Point3f p = {sum.x / calibrationPoints.size(),
                         sum.y / calibrationPoints.size(),
                         sum.z / calibrationPoints.size()};
    calibrationPoints.clear();
    return p;
}