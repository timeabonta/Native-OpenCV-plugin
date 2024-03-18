#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// 'C' típusú interfész definiálása
extern "C" {

    // OpenCV verzióját visszaadó függvény
    __attribute__((visibility("default"))) __attribute__((used)) const char* version() {
        return CV_VERSION;
    }

    __attribute__((visibility("default"))) __attribute__((used))
    const char* processImage(const char* imagePath) {
        Mat image = imread(imagePath, IMREAD_COLOR);
        if(image.empty()) {
            return "Failed to load image";
        }

        Mat grayImage;
        cvtColor(image, grayImage, COLOR_BGR2GRAY);

        string result = "Image processed successfully. Size: ";
        result += to_string(grayImage.rows) + "x" + to_string(grayImage.cols) + ", Type: " + to_string(grayImage.type());

        return result.c_str();
    }

    __attribute__((visibility("default"))) __attribute__((used))
    const char* detectAndFrameObjects(const char* imagePath) {
        static string lastError;
        Mat src = imread(imagePath, IMREAD_COLOR);
        if(src.empty()) {
            lastError = "Failed to load image";
            return lastError.c_str();
        }

        // szin szerinti keretezes
        Mat hsvImage;
        cvtColor(src, hsvImage, COLOR_BGR2HSV);

        Mat hsv[3];
        split(hsvImage, hsv);
        int histSize = 256;
        float range[] = {0, 256};
        const float* histRange = {range};
        Mat hist;
        calcHist(&hsv[0], 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

        // legyakoribb szinarnyalat indexe
        double maxVal = 0;
        Point maxValPoint;
        minMaxLoc(hist, 0, &maxVal, 0, &maxValPoint);
        int dominantHueIndex = maxValPoint.y;

        int hueTolerance = 10; // tolerancia a dominans szinarnyalat korul
        Scalar lowerBound(dominantHueIndex - hueTolerance, 50, 50);
        Scalar upperBound(dominantHueIndex + hueTolerance, 255, 255);
        Mat mask;
        inRange(hsvImage, lowerBound, upperBound, mask);
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        //legnagyobb kontur -> legnagyobb targy
        double maxArea = 0;
        vector<Point> largestContour;
        for (const vector<Point>& contour : contours) {
            double area = contourArea(contour);
            if (area > maxArea) {
                maxArea = area;
                largestContour = contour;
            }
        }

        if (!largestContour.empty()) {
            Rect boundingBox = boundingRect(largestContour);
            rectangle(src, boundingBox, Scalar(0, 255, 0), 2);
        }

        string resultStream = "/data/data/com.example.himo/files/detected.png";
        if (!imwrite(resultStream, src)) {
            lastError = "Failed to save output image";
            return lastError.c_str();
        }

        return strdup(resultStream.c_str());
    }
}
