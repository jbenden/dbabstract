#!/bin/sh

find . -name CMakeCache.txt -delete
find . -name cmake_install.cmake -delete
find . -name CMakeFiles -exec rm -fr {} \;
find . -name Makefile -delete
rm -f src/rocket-etl tests/tests tests/CTestTestfile.cmake
rm -fr temp libs
