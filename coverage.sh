#!/bin/bash

set -e 

lcov --capture --directory test --output-file coverage.info
genhtml coverage.info --output-directory coverage
open coverage/index.html

