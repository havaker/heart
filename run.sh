#!/usr/bin/env bash

set -e # Exit on error
set -x # Print executed commands

# Create a `build` directory if it doesn't exist
mkdir -p build
cd build/

# Generate the Makefile if it doesn't exist
if [ ! -f Makefile ]; then
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
fi

# Build the project
make

# Run the project
./renderer --width 256 --height 256 --quality 1 --fps 30 --length 10 | \
    ffmpeg -hide_banner -loglevel error -f image2pipe -framerate 30 -i - -c:v libx264rgb -preset ultrafast -qp 0 -f matroska - | \
    ffplay -hide_banner -
