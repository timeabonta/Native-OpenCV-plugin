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
        Scalar lowerBlue = Scalar(90, 50, 50);
        Scalar upperBlue = Scalar(150, 255, 255);
        Mat mask, result;
        inRange(hsvImage, lowerBlue, upperBlue, mask);
        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));  //zajcsokkentes
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        for (const auto& contour : contours) {
            if (contourArea(contour) > 100) {
                Rect boundingBox = boundingRect(contour);
                rectangle(src, boundingBox, Scalar(0, 255, 0), 2);
            }
        }

        ostringstream resultStream;
        resultStream << "/data/data/com.example.himo/files/detected.png";

        if (!imwrite(resultStream.str(), src)) {
            lastError = "Failed to save output image";
            return lastError.c_str();
        }

        return strdup(resultStream.str().c_str());
    }
}
