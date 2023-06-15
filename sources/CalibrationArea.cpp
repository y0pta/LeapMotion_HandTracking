//
// Created by Liza on 6/3/2023.
//

#include "CalibrationArea.h"
#include <fstream>
#include <iostream>
#include "spdlog/spdlog.h"


std::string CalibrationArea::str(EPointType type)
{
    return _pointTypeName[type];
}

bool CalibrationArea::save(const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        spdlog::get("logger")->error("Error: Could not open file " + filename);
        std::cerr << "Error: Could not open file \"" << filename << "\"" << std::endl;
        return false;
    }

    // Write comment
    file << "# CalibrationArea points in order: "<< str(EPointType(0)) << " "
                << str(EPointType(1)) << " "
                << str(EPointType(2)) << " "
                << str(EPointType(3)) << std::endl;

    // Write calibration points
    for (int e = eTopLeft; e <= eBottomLeft; ++e) {
        file << _points[EPointType(e)].x << " "
            << _points[EPointType(e)].y << " "
            << _points[EPointType(e)].z << std::endl;
    }

    // Check for errors
    if (file.fail()) {
        spdlog::get("logger")->error("Error: Could not write calibration points to file " + filename);
        std::cerr << "Error: Could not write calibration points to file \"" << filename << "\"" << std::endl;
        return false;
    }

    file.close();
    return true;
}

bool CalibrationArea::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        spdlog::get("logger")->error("Error: Could not open file " + filename);
        std::cerr << "Error: Could not open file \"" << filename << "\"" << std::endl;
        return false;
    }

    // First line contains comment, so read and discard it
    std::string comment;
    std::getline(file, comment);

    // Read calibration points
    for (int e = eTopLeft; e <= eBottomLeft; ++e) {
        file >> _points[EPointType(e)].x >> _points[EPointType(e)].y >> _points[EPointType(e)].z;
    }

    // Check for errors
    if (file.fail()) {
        spdlog::get("logger")->error("Error: Could not read calibration points from file " + filename);
        std::cerr << "Error: Could not read calibration points from file \"" << filename << "\"" << std::endl;
        return false;
    }

    file.close();
    return true;
}


void CalibrationArea::calibrate()
{
    /// Amend top right corner
    // suppose dot product vec(tl - tr) * vec(bl - tl) = 0.
    // We need to align tr to tr' to make equation above valid
    // Here we find the vector, where tr is supposed to be
    auto middlePoint = (_points[eTopLeft] + _points[eBottomRight]) * 0.5 ;
    auto middleVec = _points[eTopRight] - middlePoint;
    // tr' = middlePoint + t * middleVec;
    // So, dot(tl - tr', bl - tr') = 0
    // Now we've got quadratic equation:
    //||middleVec||^2 * t^2 - ||tl - br||^2 / 4 = 0
    double t1 = norm(_points[eTopLeft] - _points[eBottomRight]) / (2 * norm(_points[eTopRight] - middlePoint));
    double t2 = -t1;
    _points[eBottomLeft] = middlePoint + t2 * middleVec;
    _points[eTopRight] = middlePoint + t1 * middleVec;

    /// make sure that points are correctly calculated
    assert(norm(_points[eBottomLeft] - _points[eBottomRight]) - norm(_points[eTopLeft] - _points[eTopRight]) <= 0.001);

    /// calc average distance from camera to surface
    _averageDistance  = _points[eTopLeft].y + _points[eTopRight].y + _points[eBottomLeft].y + _points[eBottomRight].y;
    _averageDistance  /= 4.0;

    /// calc w / h of interaction surface
    _aspectRatio = norm(_points[eTopLeft] - _points[eTopRight]) / norm(_points[eTopLeft] - _points[eBottomLeft]);

    /// calc unit normal to surface and distance from origin
    // n = cross(w, h), where w and h - edge vectors
    cv::Vec3f w = _points[eTopLeft] - _points[eTopRight];
    w = normalize(w);
    cv::Vec3f h = _points[eTopLeft] - _points[eBottomLeft];
    h = normalize(h);
    _normal = w.cross(h);
    assert(fabs(norm(_normal)) - 1.0 < 0.001);
    _d = - _normal.dot(_points[eTopLeft]);
}

bool CalibrationArea::isPointInside(const cv::Point3f& point) const
{
    // calc edge vectors counterclockwise
    cv::Vec3f edge1 = _points[eTopRight] - _points[eTopLeft];
    cv::Vec3f edge2 = _points[eBottomRight] - _points[eTopRight];
    cv::Vec3f edge3 = _points[eBottomLeft] - _points[eBottomRight];
    cv::Vec3f edge4 = _points[eTopLeft] - _points[eBottomLeft];

    // calc vector from vertices to point
    cv::Vec3f pointVector1 = point - _points[eTopLeft];
    cv::Vec3f pointVector2 = point - _points[eTopRight];
    cv::Vec3f pointVector3 = point - _points[eBottomRight];
    cv::Vec3f pointVector4 = point - _points[eBottomLeft];

    return (pointVector1.dot(edge1) > 0 &&
            pointVector2.dot(edge2) > 0 &&
            pointVector3.dot(edge3) > 0 &&
            pointVector4.dot(edge4) > 0);
}

double CalibrationArea::pointToSurfaceDistance(const cv::Point3f& point) const
{
    return _normal.dot(point) + _d;
}

cv::Point3f CalibrationArea::getPointProjection(const cv::Point3f& point) const
{
    // Find orthogonal projection, using the following equations:
    // 1) dot(proj, n) + d = 0
    // 2) proj = point - n * t
    // ==> t = (d + dot(point, n)) / dot(n,n)
    // proj = point - n * t
    double t = (_d + _normal.dot(point)) / _normal.dot(_normal);
    return cv::Point3f(cv::Vec3f(point) - _normal * t);
}

cv::Point2f CalibrationArea::findPointUV(const cv::Point3f& point) const
{
    auto proj = getPointProjection(point);
    cv::Point3f w = _points[eTopRight] - _points[eTopLeft];
    cv::Point3f h = _points[eBottomLeft] - _points[eTopLeft];

    double x = (proj.x - _points[eTopLeft].x) / w.x ;
    double y = (proj.z - _points[eTopLeft].z) / h.z ;
    return cv::Point2f(x, y);
}