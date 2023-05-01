//
// Created by Liza on 5/1/2023.
//

#include "CalibrationApp.h"
#include "LeapConnection.h"
#include "CalibData.h"
#include <memory>

std::unique_ptr<CalibrationApp> __app;

/** Save stereo image in cur_image */
void imageCallback(LeapConnection& con, const LEAP_IMAGE_EVENT *imageEvent){
    if (__app && __app->isCalibrating) {
        auto &imgVoidPtr = imageEvent->image[0].data;
        auto &imgW = imageEvent->image[0].properties.width;
        auto &imgH = imageEvent->image[0].properties.height;
        cv::Mat img0 = OpencvUtils::voidPtrToMat(imgVoidPtr, imgW, imgH, 1);

        auto &imgVoidPtr1 = imageEvent->image[1].data;
        cv::Mat img1 = OpencvUtils::voidPtrToMat(imgVoidPtr1, imgW, imgH, 1);

        __app->image_mutex.lock();
        OpencvUtils::imgConcat(img0, img1, __app->curImage);
        __app->image_mutex.unlock();
    }
}

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
void frameCalibCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    if (__app && __app->recordEvent && (frame->nHands > 0)){
        __app->recordEvent = false;

        std::cout << "On frame " << frame->info.frame_id << ". Detected: " << frame->nHands << std::endl;
        auto& hand = frame->pHands[0];
        auto& coords = hand.index.distal.next_joint;

        std::cout << coords.x << " "
                  << coords.y << " "
                  << coords.z << std::endl;

        auto& points = __app->calibrationPoints;
        points.push_back(coords);

        // Check if calibration done
        if (points.size() == 4) {
            float h = (points[0].y + points[1].y + points[2].y + points[3].y) / 4.0;
            std::cout << "Calibration done. Surface distance = " << h << std::endl;
            __app->isCalibrating = false;
        }
    }
}

CalibrationApp::CalibrationApp(){
    cv::Mat curImage(100, 100, CV_8UC1, 1);
    __app = std::unique_ptr<CalibrationApp>(this);
}

CalibrationApp::~CalibrationApp() {
    __app = nullptr;
}

void CalibrationApp::run() {
    LeapConnection connection;

    connection.image_callback = imageCallback;
    connection.tracking_callback = frameCalibCallback;

    connection.open();
    bool result = _getCalibrationPoints();
    connection.close();

    if (result)
        _data.saveCalibData("calib.txt");
}

bool CalibrationApp::_getCalibrationPoints() {
    // TODO: process false case
    cv::namedWindow("Display window", cv::WINDOW_FREERATIO);

    while (isCalibrating) {
        image_mutex.lock();
        imshow("Display window", curImage);
        image_mutex.unlock();
        last_key = cv::waitKey(delay);
        // ESC key
        if (last_key == 27)
            std::exit(0);
        // Record key
        if (last_key == int('r')) {
            std::cout << "R pressed" << std::endl;
            recordEvent = true;
        }
    }
    OpencvUtils::printTextOnImage(curImage, "Calibration done.");
    imshow("Display window", curImage);

    // save calib points
    auto tl = cv::Point3f(calibrationPoints[0].x, calibrationPoints[0].y, calibrationPoints[0].z);
    auto bl = cv::Point3f(calibrationPoints[1].x, calibrationPoints[1].y, calibrationPoints[1].z);
    auto br = cv::Point3f(calibrationPoints[2].x, calibrationPoints[2].y, calibrationPoints[2].z);
    auto tr = cv::Point3f(calibrationPoints[3].x, calibrationPoints[3].y, calibrationPoints[3].z);
    _data = CalibData(tl, bl, br, tr);
    cv::waitKey(3000);

    cv::destroyAllWindows();
}