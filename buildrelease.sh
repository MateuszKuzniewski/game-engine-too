#!/bin/bash
cmake --build build --config Release && LSAN_OPTIONS=suppressions=lsan_suppressions.txt ./build/Release/get
