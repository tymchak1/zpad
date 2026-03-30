#include "FrameProcessor.hpp"
#include <sstream>
#include <iomanip>

FrameProcessor::FrameProcessor() {}

cv::Mat FrameProcessor::process(const cv::Mat& frame, Mode mode,
                                 int brightness, float zoom, double angle,
                                 bool flipH, bool flipV, bool showHUD,
                                 int frameCount, double fps) {
    cv::Mat result;

    switch (mode) {
        case Mode::INVERT:  result = applyInvert(frame);  break;
        case Mode::BLUR:    result = applyBlur(frame);     break;
        case Mode::CANNY:   result = applyCanny(frame);    break;
        case Mode::SOBEL:   result = applySobel(frame);    break;
        case Mode::BINARY:  result = applyBinary(frame);   break;
        case Mode::GLITCH:  result = applyGlitch(frame);   break;
        default:            result = frame.clone();        break;
    }

    result = applyBrightness(result, brightness);
    result = applyFlip(result, flipH, flipV);
    result = applyRotation(result, angle);
    result = applyZoom(result, zoom);

    if (mode == Mode::DRAW) drawRects(result);
    if (showHUD) drawHUD(result, mode, frameCount, fps, brightness, zoom);

    return result;
}

DrawState& FrameProcessor::getDrawState() { return drawState; }

cv::Mat FrameProcessor::applyInvert(const cv::Mat& frame) {
    cv::Mat out;
    cv::bitwise_not(frame, out);
    return out;
}

cv::Mat FrameProcessor::applyBlur(const cv::Mat& frame) {
    cv::Mat out;
    cv::GaussianBlur(frame, out, cv::Size(21, 21), 0);
    return out;
}

cv::Mat FrameProcessor::applyCanny(const cv::Mat& frame) {
    cv::Mat gray, edges, out;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 50, 150);
    cv::cvtColor(edges, out, cv::COLOR_GRAY2BGR);
    return out;
}

cv::Mat FrameProcessor::applySobel(const cv::Mat& frame) {
    cv::Mat gray, gradX, gradY, absX, absY, out;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Sobel(gray, gradX, CV_16S, 1, 0);
    cv::Sobel(gray, gradY, CV_16S, 0, 1);
    cv::convertScaleAbs(gradX, absX);
    cv::convertScaleAbs(gradY, absY);
    cv::addWeighted(absX, 0.5, absY, 0.5, 0, gray);
    cv::cvtColor(gray, out, cv::COLOR_GRAY2BGR);
    return out;
}

cv::Mat FrameProcessor::applyBinary(const cv::Mat& frame) {
    cv::Mat gray, bin, out;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, bin, 128, 255, cv::THRESH_BINARY);
    cv::cvtColor(bin, out, cv::COLOR_GRAY2BGR);
    return out;
}

cv::Mat FrameProcessor::applyGlitch(const cv::Mat& frame) {
    std::vector<cv::Mat> channels;
    cv::split(frame, channels);
    int shift = 15;
    // Shift R right, B left
    cv::Mat rShifted = cv::Mat::zeros(channels[2].size(), channels[2].type());
    cv::Mat bShifted = cv::Mat::zeros(channels[0].size(), channels[0].type());
    channels[2](cv::Rect(0, 0, frame.cols - shift, frame.rows))
        .copyTo(rShifted(cv::Rect(shift, 0, frame.cols - shift, frame.rows)));
    channels[0](cv::Rect(shift, 0, frame.cols - shift, frame.rows))
        .copyTo(bShifted(cv::Rect(0, 0, frame.cols - shift, frame.rows)));
    channels[2] = rShifted;
    channels[0] = bShifted;
    cv::Mat out;
    cv::merge(channels, out);
    return out;
}

cv::Mat FrameProcessor::applyZoom(const cv::Mat& frame, float zoom) {
    if (std::abs(zoom - 1.0f) < 0.01f) return frame;
    int cx = frame.cols / 2, cy = frame.rows / 2;
    int newW = static_cast<int>(frame.cols / zoom);
    int newH = static_cast<int>(frame.rows / zoom);
    int x = std::max(0, cx - newW / 2);
    int y = std::max(0, cy - newH / 2);
    newW = std::min(newW, frame.cols - x);
    newH = std::min(newH, frame.rows - y);
    cv::Mat cropped = frame(cv::Rect(x, y, newW, newH));
    cv::Mat out;
    cv::resize(cropped, out, frame.size());
    return out;
}

cv::Mat FrameProcessor::applyRotation(const cv::Mat& frame, double angle) {
    if (std::abs(angle) < 0.1) return frame;
    cv::Point2f center(frame.cols / 2.0f, frame.rows / 2.0f);
    cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
    cv::Mat out;
    cv::warpAffine(frame, out, rot, frame.size());
    return out;
}

cv::Mat FrameProcessor::applyFlip(const cv::Mat& frame, bool flipH, bool flipV) {
    if (!flipH && !flipV) return frame;
    cv::Mat out = frame.clone();
    if (flipH && flipV) cv::flip(out, out, -1);
    else if (flipH) cv::flip(out, out, 1);
    else cv::flip(out, out, 0);
    return out;
}

cv::Mat FrameProcessor::applyBrightness(const cv::Mat& frame, int brightness) {
    if (brightness == 0) return frame;
    cv::Mat out;
    frame.convertTo(out, -1, 1.0, brightness);
    return out;
}

void FrameProcessor::drawRects(cv::Mat& frame) {
    for (auto& r : drawState.rects) {
        cv::rectangle(frame, r.first, r.second, cv::Scalar(0, 255, 0), 2);
    }
    if (drawState.drawing) {
        cv::rectangle(frame, drawState.start, drawState.current,
                      cv::Scalar(0, 200, 0), 1);
    }
}

static const char* modeName(Mode m) {
    switch (m) {
        case Mode::NORMAL:  return "Normal";
        case Mode::INVERT:  return "Invert";
        case Mode::BLUR:    return "Blur";
        case Mode::CANNY:   return "Canny";
        case Mode::SOBEL:   return "Sobel";
        case Mode::BINARY:  return "Binary";
        case Mode::GLITCH:  return "Glitch";
        case Mode::DRAW:    return "Draw";
    }
    return "?";
}

void FrameProcessor::drawHUD(cv::Mat& frame, Mode mode, int frameCount,
                              double fps, int brightness, float zoom) {
    // Background bar
    cv::rectangle(frame, cv::Point(0, 0), cv::Point(frame.cols, 28),
                  cv::Scalar(0, 0, 0), cv::FILLED);

    std::ostringstream ss;
    ss << "Mode: " << modeName(mode)
       << " | FPS: " << std::fixed << std::setprecision(1) << fps
       << " | Frame: " << frameCount
       << " | Bright: " << brightness
       << " | Zoom: " << std::setprecision(1) << zoom << "x";

    cv::putText(frame, ss.str(), cv::Point(10, 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

    // Mean intensity
    cv::Scalar mean = cv::mean(frame);
    std::ostringstream ms;
    ms << "Mean RGB: " << std::setprecision(0)
       << mean[2] << "/" << mean[1] << "/" << mean[0];
    cv::putText(frame, ms.str(), cv::Point(10, frame.rows - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(200, 200, 200), 1);
}
