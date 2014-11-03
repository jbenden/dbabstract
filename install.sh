#!/bin/sh

cd gtest-1.7.0 && cmake . && make && cd .. && cmake . && make
