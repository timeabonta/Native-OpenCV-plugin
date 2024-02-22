#ifndef COLOR_DETECTOR_H
#define COLOR_DETECTOR_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

class ColorDetector {
public:
    ColorDetector(cv::Scalar targetColor);
    std::vector<cv::Rect> detectColor(const cv::Mat& image);

private:
    cv::Scalar targetColor;
};

#endif // COLOR_DETECTOR_H
