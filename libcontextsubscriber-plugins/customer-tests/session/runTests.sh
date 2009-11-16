#!/bin/bash

which context-listen 1>/dev/null || { echo "Install contextkit-utils to run the test" && exit 1 ; }

make -C screentoggler
export PATH=screentoggler:$PATH

# Ensure that the plugin is built and that the test program finds it
if [ -n "$COVERAGE" ]
then
    make -C coverage-build
    export CONTEXT_SUBSCRIBER_PLUGINS=coverage-build/.libs
    rm -rf coverage-build/.libs/*.gcda
else
    export CONTEXT_SUBSCRIBER_PLUGINS=../../session/.libs
    make -C ../session
fi

# Use the .context file from the plugin directory
export CONTEXT_PROVIDERS=../../session/

# Run the test
python2.5 test-fullscreen.py

if [ -n "$COVERAGE" ]
then
    echo "computing coverage"
    rm -rf coverage
    mkdir -p coverage
    lcov --directory coverage-build/.libs/ --capture --output-file coverage/all.cov
    lcov --extract coverage/all.cov '*/session/*.cpp' --output-file coverage/session.cov
    genhtml -o coverage/ coverage/session.cov
fi
