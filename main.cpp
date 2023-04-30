#include <iostream>

#include <chrono>
#include <thread>
#include "LeapConnection.h"
#include "LeapConnectionSerializer.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "Utils.h"
#include <mutex>
#include <stdlib.h>

using namespace cv;

static bool is_calibrating = false;
static bool calib_event = false;
static std::mutex mutex_calib;
static int last_key = 0;
static float delay = 10;
static float table_dist;
static cv::Mat cur_image(100, 100, CV_8UC1, 1);
static cv::Mat display(100, 100, CV_8UC3, Scalar(1,1,1));
static std::mutex mutex;

static int w, h;
static std::vector<LEAP_VECTOR> calib_points; // from top-left to top-right counterclockwise
static cv::Point2f topLeft, bottomLeft, bottomRight, topRight;
static int disp_x, disp_y = 0;
static float last_h = 300;
static float tap_threshold = 10;
static bool press_event = false;
static bool move_event = false;

/** Match coords on index finger with display */
bool attachHandToDisplay(LEAP_VECTOR& coords3D, int& disp_x, int& disp_y, float& z){
    // x - left, y - down, z - on me
    cv::Point2f point(coords3D.x, coords3D.z);

    float threshold = 150;
    // check height
    if (fabs(table_dist - coords3D.y) > threshold)
        return false;

    // check if coord inside calib parallelogram
    if (!OpencvUtils::isPointInsideParallelogram(point, topLeft, bottomLeft, bottomRight, topRight))
        return false;

    // calc coords
    auto basis_x = cv::Point2f(bottomLeft - topLeft);
    auto basis_y = cv::Point2f(topRight - topLeft);

    cv::Mat A = (cv::Mat_<float>(2, 2) << basis_x.x,basis_y.x,
                                          basis_x.y, basis_y.y);
    cv::Mat A_inv;
    cv::invert(A, A_inv);
    cv::Mat point_wc = (cv::Mat_<float>(2, 1) << point.x - topLeft.x, point.y - topLeft.y);
    cv::Mat point_p = A_inv * point_wc;
    disp_x = point_p.at<float>(0,0) * h;
    disp_y = point_p.at<float>(1,0) * w;
    //std::cout << "Point attached to screen:" << x << " " << y <<std::endl;

    z = coords3D.y;
    return true;
}

/** Save stereo image in cur_image */
void imageCallback(LeapConnection& con, const LEAP_IMAGE_EVENT *imageEvent){
    if (is_calibrating) {
        auto &imgVoidPtr = imageEvent->image[0].data;
        auto &imgW = imageEvent->image[0].properties.width;
        auto &imgH = imageEvent->image[0].properties.height;
        Mat img0 = OpencvUtils::voidPtrToMat(imgVoidPtr, imgW, imgH, 1);

        auto &imgVoidPtr1 = imageEvent->image[1].data;
        Mat img1 = OpencvUtils::voidPtrToMat(imgVoidPtr1, imgW, imgH, 1);

        mutex.lock();
        OpencvUtils::imgConcat(img0, img1, cur_image);
        mutex.unlock();
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
    if (calib_event && is_calibrating && (frame->nHands > 0)){
        mutex_calib.lock();
        calib_event = false;
        mutex_calib.unlock();

        std::cout << "On frame " << frame->info.frame_id << ". Detected: " << frame->nHands << std::endl;
        auto& hand = frame->pHands[0];
        auto& coords = hand.index.distal.next_joint;

        std::cout << coords.x << " "
                  << coords.y << " "
                  << coords.z << std::endl;

        calib_points.push_back(coords);

        // if calibration done
        if (calib_points.size() == 4) {
            float h = (calib_points[0].y + calib_points[1].y + calib_points[2].y + calib_points[3].y) / 4.0;
            std::cout << "Calibration done. Surface distance = " << h << std::endl;
            is_calibrating = false;
        }
    }
}

/** Match 3D points with 2d display*/
void frameDisplayCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    if (frame->nHands > 0) {
        for (int i = 0; i < frame->nHands; ++i) {
            auto &hand = frame->pHands[i];
            int x, y;
            if (attachHandToDisplay(hand.index.distal.next_joint, x, y, last_h)) {
                mutex.lock();
                disp_x = x;
                disp_y = y;
                move_event = true;
                if (fabs(last_h - table_dist) < tap_threshold)
                    press_event = true;
                mutex.unlock();
            }
        }
    }
}

/** Main func for calibration */
void getCalibPoints(){
    cv::namedWindow("Display window", cv::WINDOW_FREERATIO);

    while (is_calibrating) {
        mutex.lock();
        imshow("Display window", cur_image);
        mutex.unlock();
        last_key = cv::waitKey(delay);
        // ESC key
        if (last_key == 27)
            std::exit(0);
        if (last_key == int('r')) {
            std::cout << "R pressed" << std::endl;
            mutex_calib.lock();
            calib_event = true;
            mutex_calib.unlock();
        }
    }
    OpencvUtils::printTextOnImage(cur_image, "Calibration done.");
    imshow("Display window", cur_image);
    cv::waitKey(3000);


    destroyAllWindows();
}

/** Main function for display cursor depending on hand pose*/
void displayHandCursor(std::vector<LEAP_VECTOR>& calib_points){
    display = cv::Mat(h, w, CV_8UC3, cv::Scalar(255, 255, 255));

    cv::namedWindow("Display window", cv::WINDOW_FREERATIO);

    delay = 1000 / 40.0;
    while (cv::waitKey(delay) != 27) {
        display = cv::Mat(h, w, CV_8UC3, cv::Scalar(255, 255, 255));
        mutex.lock();
        if (move_event) {
            circle(display, Point(disp_y, disp_x), 20, cv::Scalar(0, 0, 255), 2);
            move_event = false;
        }
        if (press_event) {
            circle(display, Point(disp_y, disp_x), 40, cv::Scalar(0, 255, 0), 5);
            press_event = false;
        }
        mutex.unlock();
        cv::imshow("Display window", display);
    }
    destroyAllWindows();
}

/** Calculate w, h, table height */
void calibrate(){
    table_dist = (calib_points[0].y + calib_points[1].y + calib_points[2].y + calib_points[3].y) / 4.0;

    topLeft = cv::Point2f(calib_points[0].x, calib_points[0].z);
    bottomLeft = cv::Point2f(calib_points[1].x, calib_points[1].z);
    bottomRight = cv::Point2f(calib_points[2].x, calib_points[2].z);
    topRight = cv::Point2f(calib_points[3].x, calib_points[3].z);

    cv::Vec2f vertical = topRight - topLeft;
    cv::Vec2f horizontal = bottomLeft - topLeft;

    float width = cv::norm(vertical);
    float height = cv::norm(horizontal);

    w = 500.0 * width / height;
    h = 500.0;
}

int main() {
    LeapConnection connection;
    /** callbacks for image and tracking events
     *
     * Tracking callbacks:
     * frameCalibCallback - for retrieving corner points of surface
     * frameCalibCallbackDebug - cout coords for index finger
     * frameDisplayCallback - match 3D points with 2d display
     *
     * Image callbacks:
     * imageCallback - capture stereo*/

    //connection.tracking_callback = frameCalibCallbackDebug;
    connection.image_callback = imageCallback;
    connection.tracking_callback = frameDisplayCallback;

    /** set true for calibration */
    is_calibrating = false;

    /** Load calibration 3D points*/
    calib_points = LeapVectorSerialization::readCalibPoints("calib.txt");
    calibrate();
    std::cout << "Screen w, h:" << w << " " << h << std::endl;

    connection.open();

    //getCalibPoints();
    // LeapVectorSerialization::writeCalibPoints(calib_points, "calib.txt");

    displayHandCursor(calib_points);

    connection.close();
    // for saving data from camera
//    LeapConnectionSerializer::serialize("info.txt", connection);
//     LeapConnectionSerializer::saveImages(connection);
    return 0;
}
