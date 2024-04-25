#include "ColorDetector.h"

ColorDetector::ColorDetector(int hue, int hueTolerance) : m_hue(hue), m_hueTolerance(hueTolerance) {}

vector<Point> ColorDetector::findLargestObject(const Mat& frame, int hue) {

    Mat hsv, mask;
    // Átalakítas HSV színtérbe
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    int tolerance1 = 10;
    int tolerance2 = 40;
    Scalar lowerBound, upperBound;
    if (hue - tolerance1 < 0) {
        // Alsó határ negatív, két tartomány kezelése szükséges
        Scalar lowerBound1 = Scalar(0, 50, 50);
        Scalar upperBound1 = Scalar(hue + tolerance2, 255, 255);
        Scalar lowerBound2 = Scalar(180 + (hue - tolerance1), 50, 50);
        Scalar upperBound2 = Scalar(180, 255, 255);
        Mat mask1, mask2;
        inRange(hsv, lowerBound1, upperBound1, mask1);
        inRange(hsv, lowerBound2, upperBound2, mask2);
        mask = mask1 | mask2;
    } else if (hue + tolerance2 > 179) {
        // Felső határ túllépi a 180-at, két tartomány kezelése szükséges
        Scalar lowerBound1 = Scalar(hue - tolerance1, 50, 50);
        Scalar upperBound1 = Scalar(179, 255, 255);
        Scalar lowerBound2 = Scalar(0, 50, 50);
        Scalar upperBound2 = Scalar((hue + tolerance2) - 180, 255, 255);
        Mat mask1, mask2;
        inRange(hsv, lowerBound1, upperBound1, mask1);
        inRange(hsv, lowerBound2, upperBound2, mask2);
        mask = mask1 | mask2;
    } else {
        // Normál tartomány kezelése
        lowerBound = Scalar(hue - tolerance1, 50, 50);
        upperBound = Scalar(hue + tolerance2, 255, 255);
        inRange(hsv, lowerBound, upperBound, mask);
    }

    //Scalar lowerBlue = Scalar(90, 50, 50);
    //Scalar upperBlue = Scalar(150, 255, 255);

    //inRange(hsv, lowerBlue, upperBlue, mask);
    // Kontúrok keresése
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    return findLargestContour(contours);
}

vector<Point> ColorDetector::findLargestContour(const vector<vector<Point>>& contours) {
    double maxArea = 200;
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
