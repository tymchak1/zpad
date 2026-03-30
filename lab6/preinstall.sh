#!/bin/bash
set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    sudo apt update
    sudo apt install -y libopencv-dev cmake g++ make
elif [[ "$OSTYPE" == "darwin"* ]]; then
    brew install opencv cmake
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo "Done. Dependencies installed."
