//
// Created by Liza on 6/9/2023.
//

#include "GestureRecognizer.h"
#include <spdlog/spdlog.h>

FingersContainer::FingersData& FingersContainer::add(cv::Point3f& indexCoords,
                                                     cv::Point3f& middleCoords,
                                                     double interdigital_dist,
                                                     int64_t timestamp)
{
    if (initial_time == 0)
        initial_time = timestamp / 1000.0; // to millisec
    FingersData d;
    d.index = indexCoords;
    d.middle = middleCoords;
    d.time = timestamp / 1000.0 - initial_time;
    d.interdigital_dist = interdigital_dist;

    // smooth movement
//    if (data.size() > 1){
//        FingersContainer::FingersData& last = back();
//        FingersContainer::FingersData& pre_last = back(1);
//        d.index = 0.1 * pre_last.index + 0.4 * last.index + 0.5 * d.index;
//    }

    mutex.lock();
    if (data.size() >= max_count)
        data.pop_front();
    data.push_back(std::move(d));
    auto& ret = data.back();
    mutex.unlock();
    return ret;
}

void FingersContainer::clear()
{
    mutex.lock();
    data.clear();
    initial_time = 0;
    mutex.unlock();
}

FingersContainer::FingersData& FingersContainer::back()
{
    assert(data.size() > 0);
    mutex.lock();
    auto& last = data.back();
    mutex.unlock();
    return last;
}

FingersContainer::FingersData& FingersContainer::back(int i)
{
    assert(data.size() > i);
    auto it = data.rbegin();
    for (int j = i; j > 0; j--)
        it++;
    return *it;
}

cv::Point3f GestureRecognizer::mapFingerPosition(cv::Point3f& coords3D)
{
    // x - left, y - down, z - on me
    cv::Point2f screenXY = _calibration.findPointUV(coords3D);// * USHRT_MAX;
    double d = fabs(_calibration.pointToSurfaceDistance(coords3D));
    return cv::Point3f(screenXY.x * screenW, screenXY.y * screenH, d);
}

std::pair<EGestureType, cv::Point2f> GestureRecognizer::recognize(cv::Point3f& indexCoords, cv::Point3f& middleCoords, int64_t timestamp){
    auto index = mapFingerPosition(indexCoords);
    auto middle = mapFingerPosition(middleCoords);
    double interdigital_dist = norm(indexCoords - middleCoords);

    auto& current = _fingersContainer.add(index, middle, interdigital_dist, timestamp);
    cv::Point2f coords2D(current.index.x, current.index.y);
    if (_fingersContainer.size() < 2) return std::pair{eNone, coords2D};
    auto& last = _fingersContainer.back(1);

    // press event
    if (!press_event && last.index.z > press_threshold && current.index.z < press_threshold){
        // detect right press with two fingers
        if (!right_press_event && current.interdigital_dist < interdigital_threshold)
        {
            right_press_event = true;
            return std::pair{eRightDown, coords2D};
        }
        // detect doubleclick
        if (current.time - last_release_time < max_doubleclick_delay)
        {
            return std::pair{eDoubleClick, coords2D};
        }
        // detect single left
        press_event = true;
        return std::pair{eLeftDown, coords2D};
    }
    // release
    if (last.index.z < press_threshold && current.index.z > press_threshold){
        // detect right release
        if (right_press_event)
        {
            right_press_event = false;
            return std::pair{eRightUp, coords2D};
        }
        if (press_event)
        {
            press_event = false;
            // detect single left release
            last_release_time = current.time;
            return std::pair{eLeftUp, coords2D};
        }
    }
    // move
    if (last.index.z < interaction_threshold)
    {
        return std::pair{eMove, coords2D};
    }
    return std::pair{eNone, coords2D};
}