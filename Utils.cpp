//
// Created by Liza on 4/27/2023.
//

#include "Utils.h"
#include <fstream>

namespace LeapStatus{
    std::string toString(eLeapRS code) {
        switch (code) {
            case eLeapRS_Success:
                return "Success";
            case eLeapRS_UnknownError:
                return "Unknown error";
            case eLeapRS_InvalidArgument:
                return "Invalid argument";
            case eLeapRS_InsufficientResources:
                return "Insufficient resources";
            case eLeapRS_InsufficientBuffer:
                return "Insufficient buffer";
            case eLeapRS_Timeout:
                return "Timeout";
            case eLeapRS_NotConnected:
                return "Not connected";
            case eLeapRS_HandshakeIncomplete:
                return "Handshake incomplete";
            case eLeapRS_BufferSizeOverflow:
                return "Buffer size overflow";
            case eLeapRS_ProtocolError:
                return "Protocol error";
            case eLeapRS_InvalidClientID:
                return "Invalid client ID";
            case eLeapRS_UnexpectedClosed:
                return "Unexpected connection closed";
            case eLeapRS_UnknownImageFrameRequest:
                return "Unknown image frame request";
            case eLeapRS_UnknownTrackingFrameID:
                return "Unknown tracking frame ID";
            case eLeapRS_RoutineIsNotSeer:
                return "Routine is not seer";
            case eLeapRS_TimestampTooEarly:
                return "Timestamp too early";
            case eLeapRS_ConcurrentPoll:
                return "Concurrent poll";
            case eLeapRS_NotAvailable:
                return "Not available";
            case eLeapRS_NotStreaming:
                return "Not streaming";
            case eLeapRS_CannotOpenDevice:
                return "Cannot open device";
            case eLeapRS_Unsupported:
                return "Unsupported request";
            default:
                return "Unknown error code";
        }
    }
}
namespace LeapDeviceStatus {
    std::string toString(int deviceStatus) {
        switch (deviceStatus) {
            case eLeapDeviceStatus_Streaming:
                return "eLeapDeviceStatus_Streaming";
            case eLeapDeviceStatus_Paused:
                return "eLeapDeviceStatus_Paused";
            case eLeapDeviceStatus_Robust:
                return "eLeapDeviceStatus_Robust";
            case eLeapDeviceStatus_Smudged:
                return "eLeapDeviceStatus_Smudged";
            case eLeapDeviceStatus_LowResource:
                return "eLeapDeviceStatus_LowResource";
            case eLeapDeviceStatus_UnknownFailure:
                return "eLeapDeviceStatus_UnknownFailure";
            case eLeapDeviceStatus_BadCalibration:
                return "eLeapDeviceStatus_BadCalibration";
            case eLeapDeviceStatus_BadFirmware:
                return "eLeapDeviceStatus_BadFirmware";
            case eLeapDeviceStatus_BadTransport:
                return "eLeapDeviceStatus_BadTransport";
            case eLeapDeviceStatus_BadControl:
                return "eLeapDeviceStatus_BadControl";
            default:
                return "Unknown eLeapDeviceStatus";
        }
    }
}

namespace LeapVectorSerialization{
    std::vector<LEAP_VECTOR> readCalibPoints(const std::string& filename) {
        std::ifstream file(filename);
        std::vector<LEAP_VECTOR> calib_points;
        if (!file.is_open()) {
            std::cerr << "Error: could not open file \"" << filename << "\"\n";
        }
        else{
            // Read the first line as a comment
            std::string comment;
            std::getline(file, comment);

            // read points
            while (file.good()) {
                LEAP_VECTOR point;
                file >> point.x >> point.y >> point.z;
                calib_points.push_back(point);
            }
            file.close();
        }
        return calib_points;
    }

    void writeCalibPoints(const std::vector<LEAP_VECTOR>& points, const std::string& filename) {
        std::ofstream file(filename);

        if (file.is_open()) {
            // Write the order of the values as a comment in the first line
            file << "# There are 4 calibration points. Order: x y z\n";

            // Loop through each point and write the values to the file
            for (const auto& point : points) {
                file << point.x << " " << point.y << " " << point.z << "\n";
            }
            file.close();
        } else {
            std::cerr << "Unable to open file: " << filename << std::endl;
        }
    }
}

namespace OpencvUtils {
    cv::Mat voidPtrToMat(void *data, int width, int height, int channels) {
        return cv::Mat(height, width, CV_MAKETYPE(CV_8U, channels), data);
    }

    void imgConcat(cv::Mat& img1, cv::Mat& img2, cv::Mat& combined){
        // create a black image to put the two images side by side
        combined = cv::Mat::zeros(img1.rows, img1.cols + img2.cols, CV_8UC1);

        // put the first image on the left
        cv::Mat roi1(combined, cv::Rect(0, 0, img1.cols, img1.rows));
        img1.copyTo(roi1);

        // put the second image on the right
        cv::Mat roi2(combined, cv::Rect(img1.cols, 0, img2.cols, img2.rows));
        img2.copyTo(roi2);
    }

    void printTextOnImage(cv::Mat& img, const std::string& text) {
        int fontFace = cv::FONT_HERSHEY_PLAIN;
        double fontScale = 1;
        int thickness = 1;

        // Get the size of the text box
        cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, nullptr);

        // Calculate the position of the text box at the center of the image
        int x = (img.cols - textSize.width) / 2;
        int y = (img.rows + textSize.height) / 2;

        // Draw the text box
        cv::putText(img, text, cv::Point(x, y), fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
    }

}
