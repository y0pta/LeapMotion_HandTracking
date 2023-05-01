//
// Created by Liza on 5/1/2023.
//

#include "TrackingApp.h"
#include <memory>
#include "LeapConnection.h"

std::unique_ptr<TrackingApp> __app;

/** Match 3D points with 2d display*/
void frameDisplayCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    if (frame->nHands > 0) {
        for (int i = 0; i < frame->nHands; ++i) {
            auto &hand = frame->pHands[i];

            if (__app->_attachHandToScreen(hand.index.distal.next_joint)) {
                __app->move_event = true;
                auto table_h = __app->_calibration.tableDist();
                if (fabs(__app->last_h - table_h) < __app->tap_threshold)
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

    _calibration = CalibData("calib.txt");
    _calibration.setInteractionAreaDistance(100);

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
            circle(display_image, cv::Point(y, x), 20, cv::Scalar(0, 0, 255), 2);
            move_event = false;
        }
        if (press_event) {
            circle(display_image, cv::Point(y, x), 40, cv::Scalar(0, 255, 0), 5);
            press_event = false;
        }

        cv::imshow("Display window", display_image);
    }
    cv::destroyAllWindows();
}

bool TrackingApp::_attachHandToScreen(LEAP_VECTOR& coords3D){
    // x - left, y - down, z - on me

    // check if coord inside interaction area
    if (!_calibration.isInsideInteractionArea(cv::Point3f(coords3D.x, coords3D.y, coords3D.z)))
        return false;

    // calc coords
    cv::Point2f point(coords3D.x, coords3D.z);

    auto bottomLeft = _calibration.bottomLeft2D();
    auto topLeft = _calibration.topLeft2D();
    auto topRight = _calibration.topRight2D();

    auto basis_x = cv::Point2f(bottomLeft - topLeft);
    auto basis_y = cv::Point2f(topRight - topLeft);

    cv::Mat A = (cv::Mat_<float>(2, 2) << basis_x.x,basis_y.x, basis_x.y, basis_y.y);
    cv::Mat A_inv;
    cv::invert(A, A_inv);
    cv::Mat point_wc = (cv::Mat_<float>(2, 1) << point.x - topLeft.x, point.y - topLeft.y);
    cv::Mat point_p = A_inv * point_wc;
    x = point_p.at<float>(0,0) * h;
    y = point_p.at<float>(1,0) * w;
    //std::cout << "Point attached to screen:" << x << " " << y <<std::endl;

    last_h = coords3D.y;
    return true;
}