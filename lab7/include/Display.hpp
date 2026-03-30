#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Display {
public:
    Display(const std::string& windowName = "Lab 6 - OpenCV");
    void show(const cv::Mat& frame);
    const std::string& getWindowName() const;

private:
    std::string windowName;
};
