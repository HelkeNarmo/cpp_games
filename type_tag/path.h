#pragma once

#include "vector2.h"
#include <vector>

class Path {
public:
    Path(const std::vector<Vector2> & pointList) {
        this->pointList = pointList;
        for (size_t i = 1; i < pointList.size(); ++i) {
            float len = (pointList[i] - pointList[i-1]).length();
            totalLength += len;
            segmentLenList.push_back(len);
        }
    }

    ~Path() = default;

    Vector2 getPositionByProgress(float progress) const {
        if (progress <= 0) return pointList.front();
        if (progress >= 1) return pointList.back();
        float targetDis = totalLength * progress;
        float accumulatedLen = 0.0f;
        for (size_t i = 1; i < pointList.size(); ++i) {
            accumulatedLen += segmentLenList[i-1];
            if (accumulatedLen >= targetDis) {
                float segmentProgress = (targetDis - (accumulatedLen - segmentLenList[i-1])) / segmentLenList[i-1];
                return pointList[i-1] + (pointList[i] - pointList[i-1]) * segmentProgress;
            }
        }
        return pointList.back();
    }

private:
    float totalLength = 0.0f;
    std::vector<Vector2> pointList;
    std::vector<float> segmentLenList;
};