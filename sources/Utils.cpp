//
// Created by Liza on 4/27/2023.
//

#include "Utils.h"
#include <fstream>
#include <conio.h>

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

    bool isPointInsideParallelogram(const cv::Point2f& point, const cv::Point2f& v1, const cv::Point2f& v2, const cv::Point2f& v3, const cv::Point2f& v4){
        // Calculate the vectors for the edges of the parallelogram
        cv::Point2f edge1 = v2 - v1;
        cv::Point2f edge2 = v3 - v2;
        cv::Point2f edge3 = v4 - v3;
        cv::Point2f edge4 = v1 - v4;

        // Calculate the vectors from the vertices to the point
        cv::Point2f pointVector1 = point - v1;
        cv::Point2f pointVector2 = point - v2;
        cv::Point2f pointVector3 = point - v3;
        cv::Point2f pointVector4 = point - v4;

        // Calculate the cross products of each edge vector and point vector
        float crossProduct1 = edge1.cross(pointVector1);
        float crossProduct2 = edge2.cross(pointVector2);
        float crossProduct3 = edge3.cross(pointVector3);
        float crossProduct4 = edge4.cross(pointVector4);

        // If the signs of the cross products are all the same, the point is inside the parallelogram
        if ((crossProduct1 >= 0 && crossProduct2 >= 0 && crossProduct3 >= 0 && crossProduct4 >= 0) ||
            (crossProduct1 <= 0 && crossProduct2 <= 0 && crossProduct3 <= 0 && crossProduct4 <= 0))
        {
            return true;
        }

        // If the signs of the cross products are not all the same, the point is outside the parallelogram
        return false;
    }
}

namespace AppUtils {
    /** input-key function */
    void waitForKeyPress(const std::string &message) {
        std::cout << message << std::endl;
        while (true) {
            //std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (_kbhit())
            {
                char key = _getch();
                if (key == 'R' || key == 'r')
                    break;
                if (key == 27 || key == 'q' || key == 'Q')
                    exit(0);
            }
        }
    }

    void showTimer(int seconds)
    {
        for (int i = seconds * 10; i >= 0; --i)
        {
            std::cout << "Elapsed time: " << int(i / 10) << ":" << i % 10  << "0 seconds\r";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << std::endl;
    }
}
