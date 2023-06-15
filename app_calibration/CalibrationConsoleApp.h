//
// Created by Liza on 6/2/2023.
//

#ifndef LEAPC_TEST_CALIBRATIONCONSOLEAPP_H
#define LEAPC_TEST_CALIBRATIONCONSOLEAPP_H
#include "../sources/LeapConnection.h"
#include "../sources/CalibrationArea.h"
#include <string>
#include <array>

class CalibrationConsoleApp {
    public:
        CalibrationConsoleApp();
        ~CalibrationConsoleApp();
        void run();
        void runDebug();

    public:
        LeapConnection _connection;
        std::vector<LEAP_VECTOR> calibrationPoints;
        std::atomic_bool recordEvent = false;
        std::atomic_bool handDetected = false;


    private:
        cv::Point3f _getCalibrationPoint(CalibrationArea::EPointType point);
};


#endif //LEAPC_TEST_CALIBRATIONCONSOLEAPP_H
