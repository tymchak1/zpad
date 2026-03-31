#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

class FaceDetector {
public:
    FaceDetector(const std::string& prototxt, const std::string& caffemodel);
    ~FaceDetector();

    void start();
    void stop();
    void submitFrame(const cv::Mat& frame);
    std::vector<cv::Rect> getFaces();

private:
    void workerLoop();

    cv::dnn::Net net;
    std::thread worker;
    std::mutex mtx;
    std::atomic<bool> running;
    std::atomic<bool> hasNewFrame;
    cv::Mat inputFrame;
    std::vector<cv::Rect> faces;
    float confidenceThreshold = 0.5f;
};
