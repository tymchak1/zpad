#include "FaceDetector.hpp"

FaceDetector::FaceDetector(const std::string& prototxt,
                           const std::string& caffemodel)
    : running(false), hasNewFrame(false) {
    net = cv::dnn::readNetFromCaffe(prototxt, caffemodel);
}

FaceDetector::~FaceDetector() { stop(); }

void FaceDetector::start() {
    running = true;
    worker = std::thread(&FaceDetector::workerLoop, this);
}

void FaceDetector::stop() {
    running = false;
    if (worker.joinable()) worker.join();
}

void FaceDetector::submitFrame(const cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(mtx);
    inputFrame = frame.clone();
    hasNewFrame = true;
}

std::vector<cv::Rect> FaceDetector::getFaces() {
    std::lock_guard<std::mutex> lock(mtx);
    return faces;
}

void FaceDetector::workerLoop() {
    while (running) {
        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!hasNewFrame) {
                // No new frame yet
            } else {
                frame = inputFrame.clone();
                hasNewFrame = false;
            }
        }

        if (frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        // Run inference
        cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300),
                                               cv::Scalar(104.0, 177.0, 123.0));
        net.setInput(blob);
        cv::Mat detections = net.forward();

        std::vector<cv::Rect> detected;
        int rows = detections.size[2];
        int cols = detections.size[3];
        float* data = (float*)detections.data;

        for (int i = 0; i < rows; i++) {
            float confidence = data[i * cols + 2];
            if (confidence > confidenceThreshold) {
                int x1 = static_cast<int>(data[i * cols + 3] * frame.cols);
                int y1 = static_cast<int>(data[i * cols + 4] * frame.rows);
                int x2 = static_cast<int>(data[i * cols + 5] * frame.cols);
                int y2 = static_cast<int>(data[i * cols + 6] * frame.rows);
                detected.push_back(cv::Rect(cv::Point(x1, y1), cv::Point(x2, y2)));
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            faces = detected;
        }
    }
}
