set ff=unix
#!bin/bash

qmake -spec macx-clang CS470.pro

make -j4