#include "ColorDetector.h"

ColorDetector::ColorDetector(cv::Scalar targetColor) : targetColor(targetColor) {}

std::vector<cv::Rect> ColorDetector::detectColor(const cv::Mat& image) {
    std::vector<cv::Rect> colorRegions;

    cv::Mat hsvImage;
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::Scalar lowerBound(targetColor[0] - 10, 100, 100);
    cv::Scalar upperBound(targetColor[0] + 10, 255, 255);
    cv::inRange(hsvImage, lowerBound, upperBound, mask);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        cv::Rect boundingRect = cv::boundingRect(contour);
        colorRegions.push_back(boundingRect);
    }

    return colorRegions;
}
