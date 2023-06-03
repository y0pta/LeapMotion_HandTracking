//
// Created by Liza on 4/26/2023.
//

#include "LeapConnection.h"
#include <iostream>
#include <vector>

static LeapConnection* __connection = nullptr;

/** Callback for when the connection opens. */
static void OnConnect(void){
    std::cout << "Connected.\n";
}

/** Callback for when a device is found. */
static void OnDeviceFound(const LEAP_DEVICE_INFO *props){
    __connection->device_found_callback(*__connection, props);
}

/** Callback for when a device. */
static void OnDevice(const LEAP_DEVICE *dev){
    __connection->device_callback(*__connection, dev);
}

/** Callback for when a frame of tracking data is available. */
void OnFrame(const LEAP_TRACKING_EVENT *frame) {
    __connection->tracking_callback(*__connection, frame);
}

/** Callback for when an image is available. */
static void OnImage(const LEAP_IMAGE_EVENT *imageEvent) {
    __connection->image_callback(*__connection, imageEvent);
}

/** Callback when device was lost. */
static void OnDeviceLost() {
    std::cout << "Device was lost" << std::endl;
}

void onDeviceCallback(LeapConnection& con, const LEAP_DEVICE *device){
    //con._devicePtr = std::unique_ptr<LEAP_DEVICE>(const_cast<LEAP_DEVICE*>(device));
}

void onDeviceFoundCallback(LeapConnection& con, const LEAP_DEVICE_INFO *props) {
    std::cout << "Device info received. Device: " << props->serial << std::endl;
    std::cout << "HFOV:" << props->h_fov << std::endl;
    std::cout << "VFOV:" << props->v_fov << std::endl;
    std::cout << "Max range(micrometers):" << props->range << std::endl;

    con._deviceInfo = *props;
    con._serial = std::string(props->serial, props->serial_length);
    con._deviceInfo.serial = con._serial.data();
}

void onFrameCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    //std::cout << "Frame received. ID: " << frame->info.frame_id << std::endl;

    // copy all data, including pointers
    LEAP_TRACKING_EVENT frame_deepcopy = *frame;


    for (int i = 0; i < frame->nHands; ++i)
    {
        auto idx = con._hands.size();
        con._hands.push_back(frame->pHands[i]);
        frame_deepcopy.pHands[i] = con._hands[idx];
    }
    con._trackingData.push_back(frame_deepcopy);
}

std::vector<char> copyImg(const LEAP_IMAGE& img){
    // find data size and start pos in img
    auto data_begin = reinterpret_cast<void *>(reinterpret_cast<char *>(img.data) + img.offset);
    auto size = img.properties.width * img.properties.height;
    // find
    std::vector<char> buf(size);
    std::memcpy(buf.data(), data_begin, size);
    return std::move(buf);
}

void onImageCallback(LeapConnection& con, const LEAP_IMAGE_EVENT *imageEvent){
    //std::cout << "Image received. Frame ID: " << imageEvent->info.frame_id << std::endl;

    auto image_event_copy = *imageEvent;

    //copy all data, including pointers for post-processing
    // copy images
    auto img0_copy = copyImg(image_event_copy.image[0]);
    auto img1_copy = copyImg(image_event_copy.image[1]);

    auto idx = con._images.size();
    con._images.push_back(std::move(img0_copy));
    con._images.push_back(std::move(img1_copy));

    image_event_copy.image[0].data = con._images[idx].data();
    image_event_copy.image[1].data = con._images[idx + 1].data();

    if (con._distMatrixLeftFlag) {
        con._distMatrixLeftFlag = 0;
        std::cout << "flag "<< std::endl;
        //copy distorsion grid matrices
        auto dist = new LEAP_DISTORTION_MATRIX; // LEAP_DISTORTION_MATRIX_N*LEAP_DISTORTION_MATRIX_N*2*sizeof(float)
        std::memcpy(dist, imageEvent->image[0].distortion_matrix, sizeof(LEAP_DISTORTION_MATRIX));

        con._distMatrixLeft = cv::Mat(LEAP_DISTORTION_MATRIX_N, LEAP_DISTORTION_MATRIX_N, CV_32FC2,
                                      cv::Scalar(1.0, 1.0));
        for (int i = 0; i < LEAP_DISTORTION_MATRIX_N; ++i) {
            for (int j = 0; j < LEAP_DISTORTION_MATRIX_N; ++j) {
                auto x = dist->matrix[i][j].x;
                auto y = dist->matrix[i][j].y;
                if (x <= 1 && x >= 0 && y <= 1 && y >= 0)
                    con._distMatrixLeft.at<cv::Vec2f>(j, i) = cv::Vec2f(x, y);
            }
        }
        delete dist;
    }
    con._imagesData.push_back(image_event_copy);
}


LeapConnection::LeapConnection() {    //Set callback function pointers
    ConnectionCallbacks.on_connection          = &OnConnect;
    ConnectionCallbacks.on_device              = &OnDevice;
    ConnectionCallbacks.on_device_found        = &OnDeviceFound;
    ConnectionCallbacks.on_frame               = &OnFrame;
    ConnectionCallbacks.on_image               = &OnImage;
    ConnectionCallbacks.on_device_lost         = &OnDeviceLost;

    setDefaultCallbacks();
    __connection = this;
}

void LeapConnection::open()
{
    if (_connectionPtr.get() == nullptr) {
        auto connection = OpenConnection();
        _connectionPtr = std::unique_ptr<LEAP_CONNECTION>(connection);
        LeapSetPolicyFlags(*_connectionPtr, eLeapPolicyFlag_Images, 0);
    }
}

void LeapConnection::setDefaultCallbacks() {
    device_found_callback = &onDeviceFoundCallback;
    device_callback = &onDeviceCallback;
    tracking_callback = &onFrameCallback;
    image_callback = &onImageCallback;
}

void LeapConnection::close() {
    if (_connectionPtr.get() != nullptr) {
        CloseConnection();
        DestroyConnection();
        __connection = nullptr;
    }
}

std::vector<float> LeapConnection::getCameraDistorsion(eLeapPerspectiveType camera){
    std::vector<float> coeffs(8);
    LeapDistortionCoeffs(*_connectionPtr.get(), camera, coeffs.data());
    return coeffs;
}

cv::Mat LeapConnection::getCameraIntrinsics(eLeapPerspectiveType camera){
    std::vector<float> coeffs(9);
    LeapCameraMatrix(*_connectionPtr.get(), camera, coeffs.data());
    return cv::Mat(3, 3, CV_32F, coeffs.data()).clone();
}

cv::Mat LeapConnection::getCameraExtrinsics(eLeapPerspectiveType camera){
    std::vector<float> coeffs(16);
    LeapExtrinsicCameraMatrix(*_connectionPtr.get(), camera, coeffs.data());
    return cv::Mat(4, 4, CV_32F, coeffs.data()).clone();
}

LEAP_DEVICE_INFO LeapConnection::getDeviceInfo(){
    LEAP_DEVICE_INFO info;
    info.serial = NULL;
    eLeapRS res = LeapGetDeviceInfo(*_devicePtr.get(), &info);
    return info;
}

LEAP_VECTOR LeapConnection::getPixelFrom3D(eLeapPerspectiveType camera, LEAP_VECTOR point3D){
    cv::Mat ext = getCameraExtrinsics(camera);

    float x = ext.at<float>(0,0) * point3D.x + ext.at<float>(1,0) * point3D.y + ext.at<float>(2,0) * point3D.z +
            ext.at<float>(3,0);
    float y = ext.at<float>(0,1) * point3D.x + ext.at<float>(1,1) * point3D.y + ext.at<float>(2,1) * point3D.z +
              ext.at<float>(3,1);
    float z = ext.at<float>(0,2) * point3D.x + ext.at<float>(1,2) * point3D.y + ext.at<float>(2,2) * point3D.z +
              ext.at<float>(3,2);
    float w = ext.at<float>(0,3) * point3D.x + ext.at<float>(1,3) * point3D.y + ext.at<float>(2,3) * point3D.z +
              ext.at<float>(3,3);
    cv::Vec4f world3D(x/w, y/w, z/w, w/w);

    std::vector<float> coeffsScaleOffset(16);
    LeapScaleOffsetMatrix(*_connectionPtr.get(), camera, coeffsScaleOffset.data());
    auto scaleOffsetMat = cv::Mat(4, 4, CV_32F, coeffsScaleOffset.data());

    LEAP_VECTOR pointLeap;
    pointLeap.x = scaleOffsetMat.at<float>(0,0) * x + scaleOffsetMat.at<float>(1,0) * y + scaleOffsetMat.at<float>(2,0) * z +
            scaleOffsetMat.at<float>(3,0);
    pointLeap.y = scaleOffsetMat.at<float>(0,1) * x + scaleOffsetMat.at<float>(1,1) * y + scaleOffsetMat.at<float>(2,1) * z +
                  scaleOffsetMat.at<float>(3,1);
    pointLeap.z = scaleOffsetMat.at<float>(0,2) * x + scaleOffsetMat.at<float>(1,2) * y + scaleOffsetMat.at<float>(2,2) * z +
                  scaleOffsetMat.at<float>(3,2);

    pointLeap.x = x;
    pointLeap.y = y;
    pointLeap.z = z;

    return LeapRectilinearToPixel(*_connectionPtr.get(), camera, pointLeap);
}

void LeapConnection::clearAll(){
    _images.clear();
    _trackingData.clear();
    _imagesData.clear();
    _hands.clear();
    _serial.clear();
}