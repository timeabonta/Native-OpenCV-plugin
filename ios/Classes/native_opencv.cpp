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
}
