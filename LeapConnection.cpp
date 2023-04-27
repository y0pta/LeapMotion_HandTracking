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
static void OnDevice(const LEAP_DEVICE_INFO *props){
    __connection->device_callback(*__connection, props);
}

/** Callback for when a frame of tracking data is available. */
void OnFrame(const LEAP_TRACKING_EVENT *frame) {
    __connection->tracking_callback(*__connection, frame);
}

/** Callback for when an image is available. */
static void OnImage(const LEAP_IMAGE_EVENT *imageEvent) {
    __connection->image_callback(*__connection, imageEvent);
}

void onDeviceCallback(LeapConnection& con, const LEAP_DEVICE_INFO *props) {
    std::cout << "Device info received. Device: " << props->serial << std::endl;
    std::cout << "HFOV:" << props->h_fov << std::endl;
    std::cout << "VFOV:" << props->v_fov << std::endl;
    std::cout << "Max range(micrometers):" << props->range << std::endl;

    con._deviceInfo = *props;
    con._serial = std::string(props->serial, props->serial_length);
    con._deviceInfo.serial = con._serial.data();
}

void onFrameCallback(LeapConnection& con, const LEAP_TRACKING_EVENT *frame){
    std::cout << "Frame received. ID: " << frame->info.frame_id << std::endl;

    // copy all data, including pointers
    LEAP_TRACKING_EVENT frame_deepcopy = *frame;

    auto idx = con._hands.size();
    for (int i = 0; i < frame->nHands; ++i)
    {
        con._hands.push_back(frame->pHands[i]);
        frame_deepcopy.pHands[i] = con._hands[i];
    }
    con._trackingData.push_back(std::move(frame_deepcopy));
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
    std::cout << "Image received. Frame ID: " << imageEvent->info.frame_id << std::endl;

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

    //copy calibration matrices
    idx = con._distMatrices.size();
    con._distMatrices.push_back(*(image_event_copy.image[0].distortion_matrix));
    con._distMatrices.push_back(*(image_event_copy.image[1].distortion_matrix));

    image_event_copy.image[0].distortion_matrix = &con._distMatrices[idx];
    image_event_copy.image[1].distortion_matrix = &con._distMatrices[idx + 1];

    con._imagesData.push_back(image_event_copy);
}


LeapConnection::LeapConnection() {    //Set callback function pointers
    ConnectionCallbacks.on_connection          = &OnConnect;
    ConnectionCallbacks.on_device_found        = &OnDevice;
    ConnectionCallbacks.on_frame               = &OnFrame;
    ConnectionCallbacks.on_image               = &OnImage;

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
