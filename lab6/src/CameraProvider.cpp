#include "CameraProvider.hpp"

CameraProvider::CameraProvider(int cameraIndex) {
    cap.open(cameraIndex);
}

CameraProvider::~CameraProvider() {
    if (cap.isOpened()) cap.release();
}

bool CameraProvider::isOpened() const {
    return cap.isOpened();
}

cv::Mat CameraProvider::getFrame() {
    cv::Mat frame;
    cap >> frame;
    return frame;
}
