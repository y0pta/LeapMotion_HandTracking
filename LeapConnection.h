//
// Created by Liza on 4/26/2023.
//

#ifndef LEAPC_TEST_LEAPCONNECTION_H
#define LEAPC_TEST_LEAPCONNECTION_H
#include "LeapC.h"
extern "C" {
#include "connection/ExampleConnection.h"
}
#include <memory>
#include <vector>
#include <functional>
#include "ExampleConnection.h"
#include <string>

/** Class for getting and copying working data from motion controller**/
class LeapConnection {
    public:
        LeapConnection();
        ~LeapConnection() {close();};

        // -- Connection ops --
        void open();
        void setDefaultCallbacks();
        void close();

        //-- Callbacks --
        std::function<void(LeapConnection& con)>                                         connection_callback;
        std::function<void(LeapConnection& con, const LEAP_DEVICE_INFO *device)>         device_callback;
        std::function<void(LeapConnection& con)>                                         device_lost_callback;
        std::function<void(LeapConnection& con,
                            const eLeapDeviceStatus failure_code,
                            const LEAP_DEVICE failed_device)>                            device_failure_callback;
        std::function<void(LeapConnection& con, const uint32_t current_policies)>        policy_callback;
        std::function<void(LeapConnection& con,
                            const LEAP_TRACKING_EVENT *tracking_event)>                  tracking_callback;
        std::function<void(LeapConnection& con,
                            const eLeapLogSeverity severity,
                            const int64_t timestamp,
                            const char* message)>                                        log_callback;
        std::function<void(LeapConnection& con,
                            const uint32_t requestID,
                            const bool success)>                                         config_change_callback;
        std::function<void(LeapConnection& con,
                            const uint32_t requestID,
                            LEAP_VARIANT value)>                                         config_response_callback;
        std::function<void(LeapConnection& con, const LEAP_IMAGE_EVENT *image_event)>    image_callback;
        std::function<void(LeapConnection& con,
                            const LEAP_POINT_MAPPING_CHANGE_EVENT *map_change_event)>    point_mapping_change_callback;
        std::function<void(LeapConnection& con,
                            const uint32_t requestID,
                            LEAP_VARIANT value)>                                         head_pose_callback;
        std::function<void(LeapConnection& con, const LEAP_IMU_EVENT *imu_event)>        imu_callback;
        std::function<void(LeapConnection& con,
                            const LEAP_TRACKING_MODE_EVENT *mode_event)>                 tracking_mode_callback;

        // -- Data access --
        std::vector<LEAP_TRACKING_EVENT> trackingData() const
                                        { return  _trackingData; }
        std::vector<LEAP_IMAGE_EVENT> imagesData() const
                                        { return  _imagesData; }
        LEAP_DEVICE_INFO deviceInfo() const
                                        { return _deviceInfo; }
        public: // TODO: make private or protected, add getters and setters
            std::vector<LEAP_TRACKING_EVENT> _trackingData;
            std::vector<LEAP_IMAGE_EVENT> _imagesData;
            LEAP_DEVICE_INFO _deviceInfo;

            std::vector<std::vector<char>> _images;
            std::vector<LEAP_DISTORTION_MATRIX> _distMatrices;
            std::vector<LEAP_HAND> _hands;
            std::string _serial;

    private:
        std::unique_ptr<LEAP_CONNECTION> _connectionPtr;
};


#endif //LEAPC_TEST_LEAPCONNECTION_H
