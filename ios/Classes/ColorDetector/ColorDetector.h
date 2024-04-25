#ifndef COLOR_DETECTOR_H
#define COLOR_DETECTOR_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace cv;
using namespace std;

class ColorDetector {
public:
    ColorDetector(int hue, int hueTolerance = 10);
    vector<Point> findLargestObject(const Mat& frame, int hue);

private:
    int m_hue;
    int m_hueTolerance;
    vector<Point> findLargestContour(const vector<vector<Point>>& contours);
    Scalar lowerBound();
    Scalar upperBound();
};

#endif // COLOR_DETECTOR_H
