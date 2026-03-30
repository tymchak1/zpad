#include "CameraProvider.hpp"
#include "KeyProcessor.hpp"
#include "FrameProcessor.hpp"
#include "FaceDetector.hpp"
#include "Display.hpp"
#include <iostream>
#include <chrono>

static int g_brightness = 50;
static void onBrightness(int pos, void*) { g_brightness = pos; }

static FrameProcessor* g_fp = nullptr;
static void onMouse(int event, int x, int y, int, void*) {
    if (!g_fp) return;
    auto& ds = g_fp->getDrawState();
    if (event == cv::EVENT_LBUTTONDOWN) {
        ds.drawing = true;
        ds.start = cv::Point(x, y);
        ds.current = ds.start;
    } else if (event == cv::EVENT_MOUSEMOVE && ds.drawing) {
        ds.current = cv::Point(x, y);
    } else if (event == cv::EVENT_LBUTTONUP && ds.drawing) {
        ds.drawing = false;
        ds.rects.push_back({ds.start, cv::Point(x, y)});
    }
}

int main(int argc, char** argv) {
    int camIdx = (argc > 1) ? std::atoi(argv[1]) : 0;
    CameraProvider camera(camIdx);
    if (!camera.isOpened()) {
        std::cerr << "Error: cannot open camera" << std::endl;
        return 1;
    }

    KeyProcessor keys;
    FrameProcessor processor;
    Display display("Lab 7 - OpenCV + Face Detection");

    // Face detector (background thread)
    FaceDetector faceDetector("models/deploy.prototxt",
                              "models/res10_300x300_ssd_iter_140000.caffemodel");
    faceDetector.start();

    g_fp = &processor;
    cv::setMouseCallback(display.getWindowName(), onMouse);
    cv::createTrackbar("Brightness", display.getWindowName(),
                       NULL, 100, onBrightness);
    cv::setTrackbarPos("Brightness", display.getWindowName(), g_brightness);

    int frameCount = 0;
    double fps = 0.0;
    auto prevTime = std::chrono::steady_clock::now();

    std::cout << "=== Lab 7: OpenCV + Face Detection (multithreaded) ===" << std::endl;
    std::cout << "Keys:" << std::endl;
    std::cout << "  1-8    Switch mode (Normal/Invert/Blur/Canny/Sobel/Binary/Glitch/Draw)" << std::endl;
    std::cout << "  F      Face detection mode" << std::endl;
    std::cout << "  h/v    Flip horizontal/vertical" << std::endl;
    std::cout << "  arrows Rotate (left/right) / Zoom (up/down)" << std::endl;
    std::cout << "  i      Toggle HUD" << std::endl;
    std::cout << "  ESC/q  Quit" << std::endl;

    while (true) {
        cv::Mat frame = camera.getFrame();
        if (frame.empty()) {
            cv::waitKey(30);
            continue;
        }

        frameCount++;
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - prevTime).count();
        if (elapsed > 0.0) fps = 1.0 / elapsed;
        prevTime = now;

        // Submit frame to face detector (non-blocking)
        faceDetector.submitFrame(frame);

        g_brightness = cv::getTrackbarPos("Brightness", display.getWindowName());
        int brightness = (g_brightness - 50) * 128 / 50;

        cv::Mat result = processor.process(
            frame, keys.getMode(), brightness,
            keys.getZoom(), keys.getAngle(),
            keys.isFlippedH(), keys.isFlippedV(),
            keys.showHUD(), frameCount, fps
        );

        // Draw face rectangles in FACE mode
        if (keys.getMode() == Mode::FACE) {
            auto faces = faceDetector.getFaces();
            for (auto& r : faces) {
                cv::rectangle(result, r, cv::Scalar(0, 255, 0), 2);
                cv::putText(result, "Face", cv::Point(r.x, r.y - 8),
                            cv::FONT_HERSHEY_SIMPLEX, 0.6,
                            cv::Scalar(0, 255, 0), 2);
            }
        }

        display.show(result);

        int key = cv::waitKey(1);
        if (!keys.processKey(key)) break;
    }

    faceDetector.stop();
    return 0;
}
