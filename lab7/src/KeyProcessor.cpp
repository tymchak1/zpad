#include "KeyProcessor.hpp"
#include <algorithm>

KeyProcessor::KeyProcessor()
    : mode(Mode::NORMAL), brightness(0), zoom(1.0f), angle(0.0),
      flipH(false), flipV(false), hud(true) {}

bool KeyProcessor::processKey(int key) {
    if (key == -1) return true;
    key = key & 0xFF;

    switch (key) {
        case 27: // ESC
        case 'q':
            return false;
        case '1': mode = Mode::NORMAL;  break;
        case '2': mode = Mode::INVERT;  break;
        case '3': mode = Mode::BLUR;    break;
        case '4': mode = Mode::CANNY;   break;
        case '5': mode = Mode::SOBEL;   break;
        case '6': mode = Mode::BINARY;  break;
        case '7': mode = Mode::GLITCH;  break;
        case '8': mode = Mode::DRAW;    break;
        case 'f':
        case 'F': mode = Mode::FACE;    break;
        case 'h': flipH = !flipH;       break;
        case 'v': flipV = !flipV;       break;
        case 'i': hud = !hud;           break;
        case 0x51: angle -= 5.0;        break;
        case 0x53: angle += 5.0;        break;
        case 0x52: zoom = std::min(zoom + 0.1f, 5.0f); break;
        case 0x54: zoom = std::max(zoom - 0.1f, 0.2f); break;
        default: break;
    }
    return true;
}

Mode KeyProcessor::getMode() const { return mode; }
int KeyProcessor::getBrightness() const { return brightness; }
float KeyProcessor::getZoom() const { return zoom; }
double KeyProcessor::getAngle() const { return angle; }
bool KeyProcessor::isFlippedH() const { return flipH; }
bool KeyProcessor::isFlippedV() const { return flipV; }
bool KeyProcessor::showHUD() const { return hud; }
