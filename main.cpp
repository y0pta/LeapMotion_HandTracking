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
static std::vector<LEAP_VECTOR> calib_points; // from top-left to top-right counterclockwise
static float table_dist;
static cv::Mat cur_image(100, 100, CV_8UC1, 1);
static cv::Mat display(100, 100, CV_8UC3, Scalar(1,1,1));
static std::mutex mutex;
static int w, h;

/** Match coords on index finger with display */
bool attachHandToDisplay(LEAP_HAND& hand, int& disp_x, int& disp_y ){
    // x - left, y - down, z - on me
    auto& coords = hand.index.distal.next_joint;
    cv::Point2f point(coords.x, coords.z);

    float threshold = 50;
    // check height
    if (fabs(table_dist - coords.y) > threshold)
        return false;
    // check if coord inside calib parallelogram
    cv::Point2f v1 = cv::Point2f(calib_points[3].x, calib_points[3].z) -
                        cv::Point2f(calib_points[0].x, calib_points[0].z);
    cv::Point2f v2 =  cv::Point2f(calib_points[1].x, calib_points[1].z) -
                        cv::Point2f(calib_points[0].x, calib_points[0].z);

    float crossProduct = v1.cross(v2);

    // Check if the cross product is positive or negative
    // A positive cross product means the point is on the right side of the parallelogram, negative - on the left side
    if (crossProduct > 0) {
        // Calculate the vectors from the point to the vertices of the parallelogram
        cv::Vec2f v3 = cv::Point2f(calib_points[0].x, calib_points[0].z) - point;
        cv::Vec2f v4 = cv::Point2f(calib_points[3].x, calib_points[3].z) - point;
        cv::Vec2f v5 = cv::Point2f(calib_points[1].x, calib_points[1].z) - point;

        // Calculate the cross products of the vectors
        float cross1 = v1.cross(v3);
        float cross2 = v2.cross(v4);
        float cross3 = v1.cross(v5);

        // Check if all cross products have the same sign
        if (!(cross1 > 0 && cross2 > 0 && cross3 > 0) || (cross1 < 0 && cross2 < 0 && cross3 < 0)) {
            return false;
        }
    }

    // TODO: calc coords

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
            if (attachHandToDisplay(hand, x, y)) {
                mutex.lock();
                circle(display, Point(x, y), 4, Scalar(0, 0, 1), 2);
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
    display = cv::Mat(h, w, CV_8UC3, Scalar(0,0,0));
    cv::namedWindow("Display window", cv::WINDOW_FREERATIO);

    delay = 1000 / 40.0;
    while (cv::waitKey(delay) != 27) {
        mutex.lock();
        cv::imshow("Display window", display);
        mutex.unlock();
    }
    destroyAllWindows();
}

/** Calculate w, h, table height */
void calibrate(){
    table_dist = (calib_points[0].y + calib_points[1].y + calib_points[2].y + calib_points[3].y) / 4.0;

    cv::Vec2f vertical(calib_points[3].x - calib_points[0].x,
                  calib_points[3].z - calib_points[0].z);
    cv::Vec2f horizontal(calib_points[1].x - calib_points[0].x,
                       calib_points[1].z - calib_points[0].z);
    float width = cv::norm(vertical);
    float height = cv::norm(horizontal);

    w = 500.0 * height / width;
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
    //connection.tracking_callback = frameCalibCallback;
    connection.image_callback = imageCallback;
    connection.tracking_callback = frameDisplayCallback;

    /** set true for calibration */
    is_calibrating = false;

    /** Load calibration 3D points*/
    calib_points = LeapVectorSerialization::readCalibPoints("calib.txt");
    calibrate();

    connection.open();

    // getCalibPoints();
    // LeapVectorSerialization::writeCalibPoints(calib_points, "calib.txt");

    displayHandCursor(calib_points);

    connection.close();
    // for saving data from camera
//    LeapConnectionSerializer::serialize("info.txt", connection);
//     LeapConnectionSerializer::saveImages(connection);
    return 0;
}
