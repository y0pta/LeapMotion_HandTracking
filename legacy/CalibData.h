//
// Created by Liza on 5/1/2023.
//

#ifndef LEAPC_TEST_CALIBDATA_H
#define LEAPC_TEST_CALIBDATA_H
#include "Utils.h"

class CalibData {
    public:
        CalibData() {}
        CalibData(cv::Point3f& topLeft,
                  cv::Point3f& bottomLeft,
                  cv::Point3f& bottomRight,
                  cv::Point3f& topRight);
        CalibData(const std::string& fname);

        bool loadCalibPoints(const std::string& filename);
        bool saveCalibData(const std::string& filename);

        bool calibrate(float interactionAreaHeight);

        bool isInsideInteractionArea(const cv::Point3f& point) const;
        float tableDist() const {return _tableDistance;}
        void setInteractionAreaDistance(float value) { _interactionAreaDistance = value;}
        float aspectRatio() const {return _aspectRatio;}

        cv::Point2f topLeft2D() const { return cv::Point2f(_topLeft.x, _topLeft.z); }
        cv::Point2f topRight2D() const { return cv::Point2f(_topRight.x, _topRight.z); }
        cv::Point2f bottomLeft2D() const { return cv::Point2f(_bottomLeft.x, _bottomLeft.z); }
        cv::Point2f bottomRight2D() const { return cv::Point2f(_bottomRight.x, _bottomRight.z); }


    private:
        cv::Point3f _topLeft, _bottomLeft, _bottomRight, _topRight;
        float _tableDistance;
        float _interactionAreaDistance = 150;
        float _aspectRatio;
};


#endif //LEAPC_TEST_CALIBDATA_H
