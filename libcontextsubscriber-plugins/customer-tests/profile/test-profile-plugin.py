#!/usr/bin/env python
##
## This file is part of ContextKit.
##
## Copyright (C) 2009 Nokia. All rights reserved.
##
## Contact: Marius Vollmer <marius.vollmer@nokia.com>
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public License
## version 2.1 as published by the Free Software Foundation.
##
## This library is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with this library; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
## 02110-1301 USA
##
##
##
import sys
import unittest
import os
import string
from subprocess import Popen, PIPE
import time
import signal
from time import sleep
from ContextKit.cltool import CLTool

def timeoutHandler(signum, frame):
    raise Exception('Tests have been running for too long')

def set_profile_property(property, value):
    os.system("dbus-send --dest=com.nokia.profiled --print-reply --type=method_call /com/nokia/profiled com.nokia.profiled.%s string:%s" % (property, value))
    time.sleep(1)


class ProfilePlugin(unittest.TestCase):

    def setUp(self):
        os.environ["CONTEXT_PROVIDERS"] = "."

        # Set the initial value before subscribe
        set_profile_property("set_profile", "general")

        self.context_client = CLTool("context-listen", "Profile.Name")

    def tearDown(self):

        # Restore some default values for Profile
        set_profile_property("set_profile", "general")

    def testProfileName(self):
        self.assert_(self.context_client.expect("Profile.Name = QString:\"general\""))

        # silent
        set_profile_property("set_profile", "silent")
        self.assert_(self.context_client.expect("Profile.Name = QString:\"silent\""))

        # meeting
        set_profile_property("set_profile", "meeting")
        self.assert_(self.context_client.expect("Profile.Name = QString:\"meeting\""))

if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)
    signal.signal(signal.SIGALRM, timeoutHandler)
    signal.alarm(60)
    unittest.main()
