#pragma once
#include <opencv2/opencv.hpp>

class CameraProvider {
public:
    CameraProvider(int cameraIndex = 0);
    ~CameraProvider();
    bool isOpened() const;
    cv::Mat getFrame();

private:
    cv::VideoCapture cap;
};
