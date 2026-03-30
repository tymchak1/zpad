### Опис завдання

Лабораторна робота №7 — Computer Vision та багатопотоковість у C++.
Розширення лаби 6: додано детекцію облич (ResNet-10 SSD) в окремому потоці
(std::thread + std::mutex). Відео не зависає під час інференсу.

### Архітектура

- **CameraProvider** — захоплення відео з камери (cv::VideoCapture)
- **KeyProcessor** — обробка клавіш, перемикання режимів (enum Mode)
- **FrameProcessor** — обробка кадрів (інверсія, blur, canny, sobel, binary, glitch, draw)
- **FaceDetector** — детекція облич у фоновому потоці (cv::dnn + ResNet-10)
- **Display** — відображення результату (cv::imshow)

### Інструкція із запуску

1. `./preinstall.sh` — встановлення залежностей та завантаження моделі
2. `./build.sh` — збірка
3. `./run.sh` — запуск

### Вимоги

- macOS або Linux (Ubuntu/Debian)
- Веб-камера
- CMake >= 3.10, C++17
