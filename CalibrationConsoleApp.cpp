//
// Created by Liza on 6/2/2023.
//

#include "CalibrationConsoleApp.h"
#include <iostream>
#include <numeric>
#include <thread>
#include <chrono>
CalibrationConsoleApp* __app;

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

/** input-key function */
void waitForKeyPress(const std::string& message = "Press 'R' to continue...")
{
    std::cout << message << std::endl;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        char key;
        std::cin >> key;
        if (key == 'R' || key == 'r')
            break;
        if (key == 27)
            exit(0);
    }
}

void showTimer(int seconds)
{
    for (int i = seconds; i >= 0; --i)
    {
        std::cout << "Elapsed time: " << i << " seconds\r";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << std::endl;
}

CalibrationConsoleApp::CalibrationConsoleApp(){
    __app = this;
}

CalibrationConsoleApp::~CalibrationConsoleApp(){
    __app = nullptr;
}

void CalibrationConsoleApp::runDebug()
{
    _connection.tracking_callback = frameCalibCallbackDebug;
    _connection.open();
    waitForKeyPress("Press esc to quit.");
    _connection.close();
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
    _connection.close();
}

cv::Point3f CalibrationConsoleApp::_getCalibrationPoint(CalibrationArea::EPointType point)
{
    while(true) {
        waitForKeyPress("Position your index finger to the " + _pointTypeName[point] + " corner of the screen, then press 'R'");
        if (!handDetected){
            std::cout << "Hand is not detected, try again." << std::endl;
        }
        else {
            recordEvent = true;
            break;
        }
    }
    showTimer(3);
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