//
// Created by Liza on 5/1/2023.
//

#include "CalibData.h"
#include "fstream"
#include "Utils.h"

bool CalibData::loadCalibPoints(const std::string& filename){
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file \"" << filename << "\"" << std::endl;
        return false;
    }

    // First line contains comment, so read and discard it
    std::string comment;
    std::getline(file, comment);

    // Read calibration points
    file >> _topLeft.x >> _topLeft.y >> _topLeft.z;
    file >> _bottomLeft.x >> _bottomLeft.y >> _bottomLeft.z;
    file >> _bottomRight.x >> _bottomRight.y >> _bottomRight.z;
    file >> _topRight.x >> _topRight.y >> _topRight.z;

    // Check for errors
    if (file.fail()) {
        std::cerr << "Error: Could not read calibration points from file \"" << filename << "\"" << std::endl;
        return false;
    }

    file.close();
    return true;
}

bool CalibData::saveCalibData(const std::string& filename){
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file \"" << filename << "\"" << std::endl;
        return false;
    }

    // Write comment
    file << "# Calibration points in order: topLeft bottomLeft bottomRight topRight" << std::endl;

    // Write calibration points
    file << _topLeft.x << " " << _topLeft.y << " " << _topLeft.z << std::endl;
    file << _bottomLeft.x << " " << _bottomLeft.y << " " << _bottomLeft.z  << std::endl;
    file << _bottomRight.x << " " << _bottomRight.y << " " << _bottomRight.z << std::endl;
    file << _topRight.x << " " << _topRight.y << " " << _topRight.z << std::endl;

    // Check for errors
    if (file.fail()) {
        std::cerr << "Error: Could not write calibration points to file \"" << filename << "\"" << std::endl;
        return false;
    }

    file.close();
    return true;
}

CalibData::CalibData(const std::string& fname){
    loadCalibPoints(fname);
    calibrate(_interactionAreaDistance);
}

CalibData::CalibData(cv::Point3f& topLeft, cv::Point3f& bottomLeft,
                    cv::Point3f& bottomRight,cv::Point3f& topRight) :
                    _topLeft(topLeft), _bottomLeft(bottomLeft),
                    _bottomRight(bottomRight), _topRight(topRight){
    calibrate(_interactionAreaDistance);
}

///
/// \param interactionAreaHeight
/// \return
bool CalibData::calibrate(float interactionAreaHeight) {
    _interactionAreaDistance = interactionAreaHeight;
    _tableDistance = ( _topLeft.y + _topRight.y + _bottomLeft.y + _bottomRight.y) / 4.0;

    cv::Vec2f vertical = cv::Point2f(_topRight.x, _topRight.z) - cv::Point2f(_topLeft.x, _topLeft.z);
    cv::Vec2f horizontal = cv::Point2f(_bottomLeft.x, _bottomLeft.z) - cv::Point2f(_topLeft.x, _topLeft.z);

    float width = cv::norm(vertical);
    float height = cv::norm(horizontal);
    _aspectRatio= width / height;

    return true;
}

///
/// \param point
/// \return
bool CalibData::isInsideInteractionArea(const cv::Point3f& point) const{
    // check height
    auto val = fabs( _tableDistance - point.y);
    bool result = fabs( _tableDistance - point.y) > _interactionAreaDistance;
    if (fabs( _tableDistance - point.y) > _interactionAreaDistance)
        return false;

    // check if coord inside calib parallelogram
    return OpencvUtils::isPointInsideParallelogram(cv::Point2f (point.x,point.z),
                                                 cv::Point2f(_topLeft.x, _topLeft.z),
                                                 cv::Point2f(_bottomLeft.x, _bottomLeft.z),
                                                 cv::Point2f(_bottomRight.x, _bottomRight.z),
                                                 cv::Point2f(_topRight.x, _topRight.z));
}