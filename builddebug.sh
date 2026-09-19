#!/bin/bash
cmake --build build --config Debug && LSAN_OPTIONS=suppressions=lsan_suppressions.txt ./build/Debug/get
