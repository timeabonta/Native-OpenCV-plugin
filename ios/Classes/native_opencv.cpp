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
}
