#!/bin/bash

which context-listen 1>/dev/null || { echo "Install contextkit-utils to run the test" && exit 1 ; }

PATH=$PATH:screentoggler CONTEXT_PROVIDERS=../../fullscreen/ CONTEXT_SUBSCRIBER_PLUGINS=../../fullscreen/.libs/ python2.5 test-fullscreen-plugin.py
