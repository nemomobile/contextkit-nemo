#!/bin/bash -e

which context-listen 1>/dev/null || { echo "Install contextkit-utils to run the test" && exit 1 ; }

# Ensure that the plugin is built and that the test program finds it
if [ -n "$COVERAGE" ]
then
    make -C coverage-build
    export CONTEXT_SUBSCRIBER_PLUGINS=coverage-build/.libs
    rm -rf coverage-build/.libs/*.gcda
else
    export CONTEXT_SUBSCRIBER_PLUGINS=../../profile/.libs
    make -C ../../profile
fi

# Run the tests
python2.5 test-profile-plugin.py

if [ -n "$COVERAGE" ]
then
    echo "computing coverage"
    rm -rf coverage
    mkdir -p coverage
    lcov --directory coverage-build/.libs/ --capture --output-file coverage/all.cov
    lcov --extract coverage/all.cov '*/profile/*.cpp' --output-file coverage/profile.cov
    genhtml -o coverage/ coverage/profile.cov
fi

