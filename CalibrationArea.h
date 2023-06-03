//
// Created by Liza on 6/3/2023.
//

#ifndef LEAPC_TEST_CALIBRATIONAREA_H
#define LEAPC_TEST_CALIBRATIONAREA_H
#include <array>
#include <string>
#include <opencv2/highgui.hpp>

const std::array<std::string, 4> _pointTypeName = {"TOP LEFT", "TOP RIGHT", "BOTTOM RIGHT", "BOTTOM LEFT"};

class CalibrationArea {
    public:
        enum EPointType{ eTopLeft = 0,
            eTopRight,
            eBottomRight,
            eBottomLeft};
    public:
        static std::string str(EPointType type);

    public:
        CalibrationArea(){}
        CalibrationArea(const std::string& filename) {load(filename);}
        CalibrationArea(std::array<cv::Point3f, 4>& data){ _points = data;}
        ~CalibrationArea() {}
    public:
        void calibrate();

        bool save(const std::string& filename) const;
        bool load(const std::string& filename);

        bool isPointInside(const cv::Point3f& point) const;
        cv::Point3f getPointProjection(const cv::Point3f& point) const;
        double pointToSurfaceDistance(const cv::Point3f& point) const;
        cv::Point2f findPointUV(const cv::Point3f& point) const;

        double averageDistance() const {return _averageDistance ;}
        double aspectRatio() const {return _aspectRatio;}
        void setPoint(EPointType type, const cv::Point3f& point) { _points[type] = point; }
        cv::Vec3f getPoint(EPointType type) const { return _points[type]; }

private:
        //bool _calibrated = false;
        std::array<cv::Point3f, 4> _points = {cv::Point3f(0,0,0), cv::Point3f(0,0,0), cv::Point3f(0,0,0), cv::Point3f(0,0,0)};
        double _averageDistance = 0;
        double _aspectRatio = 0;
        cv::Vec3f _normal;
        double _d = 0;
};


#endif //LEAPC_TEST_CALIBRATIONAREA_H
