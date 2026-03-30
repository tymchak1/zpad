#include "CameraProvider.hpp"
#include "KeyProcessor.hpp"
#include "FrameProcessor.hpp"
#include "Display.hpp"
#include <iostream>
#include <chrono>

// Brightness trackbar callback
static int g_brightness = 50; // trackbar value 0-100, mapped to -128..+128
static void onBrightness(int pos, void*) { g_brightness = pos; }

// Mouse callback for drawing rectangles
static FrameProcessor* g_fp = nullptr;
static void onMouse(int event, int x, int y, int /*flags*/, void*) {
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
    } else if (event == cv::EVENT_MOUSEWHEEL) {
        // Scroll up/down for zoom handled via key (not all backends support this)
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
    Display display("Lab 6 - OpenCV");

    g_fp = &processor;
    cv::setMouseCallback(display.getWindowName(), onMouse);
    cv::createTrackbar("Brightness", display.getWindowName(),
                       NULL, 100, onBrightness);
    cv::setTrackbarPos("Brightness", display.getWindowName(), g_brightness);

    int frameCount = 0;
    double fps = 0.0;
    auto prevTime = std::chrono::steady_clock::now();

    std::cout << "=== Lab 6: OpenCV Video Processing ===" << std::endl;
    std::cout << "Keys:" << std::endl;
    std::cout << "  1-8   Switch mode (Normal/Invert/Blur/Canny/Sobel/Binary/Glitch/Draw)" << std::endl;
    std::cout << "  h/v   Flip horizontal/vertical" << std::endl;
    std::cout << "  arrows Rotate (left/right) / Zoom (up/down)" << std::endl;
    std::cout << "  i     Toggle HUD" << std::endl;
    std::cout << "  ESC/q Quit" << std::endl;
    std::cout << "  Mouse: draw rectangles in Draw mode (8)" << std::endl;
    std::cout << "  Slider: brightness" << std::endl;

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

        // Map trackbar 0-100 to brightness -128..+128
        g_brightness = cv::getTrackbarPos("Brightness", display.getWindowName());
        int brightness = (g_brightness - 50) * 128 / 50;

        cv::Mat result = processor.process(
            frame, keys.getMode(), brightness,
            keys.getZoom(), keys.getAngle(),
            keys.isFlippedH(), keys.isFlippedV(),
            keys.showHUD(), frameCount, fps
        );

        display.show(result);

        int key = cv::waitKey(1);
        if (!keys.processKey(key)) break;
    }

    return 0;
}
