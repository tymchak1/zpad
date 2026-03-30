#pragma once

enum class Mode {
    NORMAL,
    INVERT,
    BLUR,
    CANNY,
    SOBEL,
    BINARY,
    GLITCH,
    DRAW,
    FACE
};

class KeyProcessor {
public:
    KeyProcessor();
    // Returns false if should quit (ESC/q)
    bool processKey(int key);
    Mode getMode() const;
    int getBrightness() const;
    float getZoom() const;
    double getAngle() const;
    bool isFlippedH() const;
    bool isFlippedV() const;
    bool showHUD() const;

private:
    Mode mode;
    int brightness;
    float zoom;
    double angle;
    bool flipH;
    bool flipV;
    bool hud;
};
