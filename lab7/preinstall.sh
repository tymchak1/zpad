#!/bin/bash
set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    sudo apt update
    sudo apt install -y libopencv-dev cmake g++ make wget
elif [[ "$OSTYPE" == "darwin"* ]]; then
    brew install opencv cmake wget
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

# Download face detection model
mkdir -p models
wget -nc -O models/deploy.prototxt \
    https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt
wget -nc -O models/res10_300x300_ssd_iter_140000.caffemodel \
    https://raw.githubusercontent.com/opencv/opencv_3rdparty/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel

echo "Done. Dependencies and models installed."
