#!/bin/bash
cmake --build build && LSAN_OPTIONS=suppressions=lsan_suppressions.txt ./build/get
