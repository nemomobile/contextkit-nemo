#!/bin/bash

which context-listen 1>/dev/null || { echo "Install contextkit-utils to run the test" && exit 1 ; }

# Ensure that the plugin is built and that the test program finds it
if [ -n "$COVERAGE" ]
then
    make -C coverage-build
    export CONTEXT_SUBSCRIBER_PLUGINS=coverage-build/.libs
    rm -rf coverage-build/.libs/*.gcda
else
    export CONTEXT_SUBSCRIBER_PLUGINS=../../bluez/.libs
    make -C ../fullscreen
fi

# Run the tests
python2.5 test-bluez-plugin.py

if [ -n "$COVERAGE" ]
then
    echo "computing coverage"
    rm -rf coverage
    mkdir -p coverage
    lcov --directory coverage-build/.libs/ --capture --output-file coverage/all.cov
    lcov --extract coverage/all.cov '*/bluez/*.cpp' --output-file coverage/bluez.cov
    genhtml -o coverage/ coverage/bluez.cov
fi

