#include "ColorDetector.h"

ColorDetector::ColorDetector(int hue, int hueTolerance) : m_hue(hue), m_hueTolerance(hueTolerance) {}

vector<Point> ColorDetector::findLargestObject(const Mat& frame) {

    Mat hsv, mask;
    // Átalakítas HSV színtérbe
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    Scalar lowerBlue = Scalar(90, 50, 50);
    Scalar upperBlue = Scalar(150, 255, 255);

    inRange(hsvImage, lowerBlue, upperBlue, mask);
    // Kontúrok keresése
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    return findLargestContour(contours);
}

vector<Point> ColorDetector::findLargestContour(const vector<vector<Point>>& contours) {
    double maxArea = 100;
    vector<Point> largestContour;
    for (const auto& contour : contours) {
        double area = contourArea(contour);
        if (area > maxArea) {
            maxArea = area;
            largestContour = contour;
        }
    }
    return largestContour;
}

Scalar ColorDetector::lowerBound() {
    return Scalar(m_hue - m_hueTolerance, 50, 50);
}

Scalar ColorDetector::upperBound() {
    return Scalar(m_hue + m_hueTolerance, 255, 255);
}
