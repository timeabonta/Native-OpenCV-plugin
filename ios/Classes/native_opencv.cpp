#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <algorithm>
#include "ColorDetector.h"

using namespace std;
using namespace cv;

static ColorDetector* detector = nullptr;

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
    void initDetector(int hue, int hueTolerance) {
        if (detector != nullptr) {
            delete detector;
            detector = nullptr;
        }

        detector = new ColorDetector(hue, hueTolerance);
    }

    __attribute__((visibility("default"))) __attribute__((used))
    void destroyDetector() {
        if (detector != nullptr) {
            delete detector;
            detector = nullptr;
        }
    }

    __attribute__((visibility("default"))) __attribute__((used))
    const float* detect(int width, int height, int rotation, uint8_t* bytes, bool isYUV, int32_t* outCount) {
        Mat frame;
        if (isYUV) {
            Mat myyuv(height + height / 2, width, CV_8UC1, bytes);
            cvtColor(myyuv, frame, COLOR_YUV2BGR_NV21);
        } else {
            frame = Mat(height, width, CV_8UC4, bytes);
        }

        Rect largestBoundingBox = boundingRect(detector->findLargestObject(frame, rotation));

        vector<float> output;
        output.push_back(largestBoundingBox.x);
        output.push_back(largestBoundingBox.y);
        output.push_back(largestBoundingBox.x + largestBoundingBox.width);
        output.push_back(largestBoundingBox.y + largestBoundingBox.height);


        *outCount = output.size();
        if (*outCount == 0) {
            return nullptr;
        }

        float* jres = (float*)malloc(sizeof(float) * output.size());
        memcpy(jres, output.data(), sizeof(float) * output.size());

        return jres;
    }


    __attribute__((visibility("default"))) __attribute__((used))
    const char* detectAndFrameObjects(const char* imagePath) {
        static string lastError;
        Mat src = imread(imagePath, IMREAD_COLOR);
        if(src.empty()) {
            lastError = "Failed to load image";
            return lastError.c_str();
        }

        //zaj csokkentes
        //GaussianBlur(src, src, Size(5, 5), 0);

        Mat hsvImage;
        cvtColor(src, hsvImage, COLOR_BGR2HSV);
        Mat hsv[3];
        split(hsvImage, hsv);
        int histSize = 256;
        float range[] = {0, 256};
        const float* histRange = {range};
        Mat hist;
        calcHist(&hsv[0], 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

        // szinarnyalatok es ezek gyakorisaganak kinyerese
        vector<pair<int, float>> hueFrequencies;
        for (int i = 0; i < histSize; ++i) {
            hueFrequencies.push_back({i, hist.at<float>(i)});
        }

        sort(hueFrequencies.begin(), hueFrequencies.end(), [](const pair<int, float>& a, const pair<int, float>& b) {
            return a.second > b.second;
        });

        // ket kulonbozo szinu targyat akarunk detektalni -> tul kozeli szinuek kiszurese
        vector<int> dominantHues;
        for (auto& hueFrequency : hueFrequencies) {
            bool tooClose = false;
            for (int selectedHue : dominantHues) {
                if (abs(hueFrequency.first - selectedHue) < 25) {
                    tooClose = true;
                    break;
                }
            }
            if (!tooClose && dominantHues.size() < 2) {
                dominantHues.push_back(hueFrequency.first);
            }
        }

        vector<Scalar> bounds;
        for (int hue : dominantHues) {
            int hueTolerance = 7;    // ezeket a parametereket meg kell modositani
            bounds.push_back(Scalar(hue - hueTolerance, 50, 50));
            bounds.push_back(Scalar(hue + hueTolerance, 255, 255));
        }

        //minden dominans szinu legnagyobb targyat kulon file-ba mentjuk
        vector<string> resultStreams;

        for (int i = 0; i < bounds.size(); i += 2) {
            Mat mask;
            Mat temp = src.clone();
            inRange(hsvImage, bounds[i], bounds[i + 1], mask);
            vector<vector<Point>> contours;
            findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            double maxArea = 200;   //a tul kicsi teruleteket elvessuk
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
                rectangle(temp, boundingBox, Scalar(255, 0, 0), 2);
                Mat objectImage = temp.clone();
                rectangle(objectImage, boundingBox, Scalar(0, 255, 0), 2);
                string resultStream = "/data/data/com.example.himo/files/detected_" + to_string(i) + ".png";
                if (!imwrite(resultStream, objectImage)) {
                    lastError = "Failed to save output image";
                    return lastError.c_str();
                }
                resultStreams.push_back(resultStream);
            }
        }

        string result = "";
        for (int i = 0; i < resultStreams.size(); ++i) {
            //a kep eleresi utja es az adott dominans szin osszefuzese
            int hue = dominantHues[i];
            result += resultStreams[i] + "," + to_string(hue) + ";";
        }

        return strdup(result.c_str());
    }
}
