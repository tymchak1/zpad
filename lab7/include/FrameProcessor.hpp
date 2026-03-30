#pragma once
#include <opencv2/opencv.hpp>
#include "KeyProcessor.hpp"

struct DrawState {
    bool drawing = false;
    cv::Point start;
    cv::Point current;
    std::vector<std::pair<cv::Point, cv::Point>> rects;
};

class FrameProcessor {
public:
    FrameProcessor();
    cv::Mat process(const cv::Mat& frame, Mode mode, int brightness,
                    float zoom, double angle, bool flipH, bool flipV,
                    bool showHUD, int frameCount, double fps);
    DrawState& getDrawState();

private:
    DrawState drawState;

    cv::Mat applyInvert(const cv::Mat& frame);
    cv::Mat applyBlur(const cv::Mat& frame);
    cv::Mat applyCanny(const cv::Mat& frame);
    cv::Mat applySobel(const cv::Mat& frame);
    cv::Mat applyBinary(const cv::Mat& frame);
    cv::Mat applyGlitch(const cv::Mat& frame);
    cv::Mat applyZoom(const cv::Mat& frame, float zoom);
    cv::Mat applyRotation(const cv::Mat& frame, double angle);
    cv::Mat applyFlip(const cv::Mat& frame, bool flipH, bool flipV);
    cv::Mat applyBrightness(const cv::Mat& frame, int brightness);
    void drawRects(cv::Mat& frame);
    void drawHUD(cv::Mat& frame, Mode mode, int frameCount, double fps,
                 int brightness, float zoom);
};
