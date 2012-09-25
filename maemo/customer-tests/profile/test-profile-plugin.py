#!/usr/bin/env python2.5
##
## This file is part of ContextKit.
##
## Copyright (C) 2010 Nokia. All rights reserved.
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
import time
import signal
import dbus
from subprocess import Popen, PIPE
from time import sleep
from ContextKit.cltool import CLTool

def timeoutHandler(signum, frame):
    raise Exception('Tests have been running for too long')

def set_profile(value):
    os.system("dbus-send --dest=com.nokia.profiled --print-reply --type=method_call /com/nokia/profiled com.nokia.profiled.set_profile string:%s" % (value))
    time.sleep(1)

def get_profile():
    bus = dbus.SessionBus()
    profiled = bus.get_object('com.nokia.profiled','/com/nokia/profiled')
    profiled_iface = dbus.Interface(profiled, dbus_interface='com.nokia.profiled')
    current_profile = profiled_iface.get_profile()
    time.sleep(1)
    return current_profile

class ProfilePlugin(unittest.TestCase):

    def setUp(self):
        # Set the initial value before subscribe
        set_profile("general")

        self.context_client = CLTool("context-listen", "Profile.Name")

    def tearDown(self):

        # Restore some default values for Profile
        set_profile("general")

    def testProfileName(self):
        self.assert_(self.context_client.expect("Profile.Name = QString:\"general\""))

        # silent
        set_profile("silent")
        self.assert_(self.context_client.expect("Profile.Name = QString:\"silent\""))

        # meeting
        set_profile("meeting")
        self.assert_(self.context_client.expect("Profile.Name = QString:\"meeting\""))

        # outdoors
        set_profile("outdoors")
        self.assert_(self.context_client.expect("Profile.Name = QString:\"outdoors\""))

    def testInvalidProfileName(self):
        self.assert_(self.context_client.expect("Profile.Name = QString:\"general\""))
        set_profile("")
        self.assertEqual(get_profile(),"general")
        self.assertFalse(self.context_client.expect("Profile.Name = QString:\"general\"", wantdump=False, timeout=10))
        self.context_client.send("value Profile.Name")
        self.assert_(self.context_client.expect("value: QString:\"general\""))

if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)
    signal.signal(signal.SIGALRM, timeoutHandler)
    signal.alarm(60)
    unittest.main()
