#!/bin/bash

which context-listen 1>/dev/null || { echo "Install contextkit-utils to run the test" && exit 1 ; }

CONTEXT_SUBSCRIBER_PLUGINS=../../bluez/.libs/ python2.5 test-bluez-plugin.py

