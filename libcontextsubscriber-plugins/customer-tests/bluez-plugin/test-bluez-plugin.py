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
## Requires python2.5-gobject and python2.5-dbus
##
import sys
import unittest
import os
import string
from subprocess import Popen, PIPE
import time
import signal
from time import sleep

def proc_kill(pid):
        os.system('../common/rec-kill.sh %d' % pid)

def timeoutHandler(signum, frame):
        raise Exception('Tests have been running for too long')

def stdoutRead (object,lines):
        list = []
        for i in range(lines):
                list.append(object.stdout.readline().rstrip())
        return list

def set_bluez_property(property, value):
        os.system('dbus-send --system --dest=org.bluez --print-reply --type=method_call /org/bluez/`pidof bluetoothd`/hci0 org.bluez.Adapter.SetProperty string:%s variant:boolean:%s' % (property, value))
        time.sleep(1)


class BluezPlugin(unittest.TestCase):

        def setUp(self):
                os.environ["CONTEXT_PROVIDERS"] = "."
                os.environ["CONTEXT_SUBSCRIBER_PLUGINS"] = "../../bluez/.libs/"
                # FIXME: this won't work in the test package...

                # Make Bluetooth invisible and un-enabled
                # Note: This test will alter the bluetooth settings of the system!

                set_bluez_property("Discoverable", "false")
                set_bluez_property("Powered", "false")

                self.context_client = Popen(["context-listen","Bluetooth.Enabled","Bluetooth.Visible"],stdin=PIPE,stdout=PIPE,stderr=PIPE)
                self.initial_output = stdoutRead(self.context_client, 2)
                self.initial_output.sort()

        def tearDown(self):

                # Restore some default values for Bluez
                set_bluez_property("Powered", "true")
                set_bluez_property("Discoverable", "false")

                proc_kill(self.context_client.pid)

        def testInitial(self):
                self.assertEqual(self.initial_output[0].strip(), "Bluetooth.Enabled = bool:false")
                self.assertEqual(self.initial_output[1].strip(), "Bluetooth.Visible = bool:false")

        def testEnabledAndVisible(self):
                # Enable
                set_bluez_property("Powered", "true")
                actual = stdoutRead(self.context_client,1)
                expected = ["Bluetooth.Enabled = bool:true"]
                self.assertEqual(actual, expected)

                # Set visible
                set_bluez_property("Discoverable", "true")
                actual = stdoutRead(self.context_client,1)
                expected = ["Bluetooth.Visible = bool:true"]
                self.assertEqual(actual, expected)

                # Set invisible
                set_bluez_property("Discoverable", "false")
                actual = stdoutRead(self.context_client,1)
                expected = ["Bluetooth.Visible = bool:false"]
                self.assertEqual(actual, expected)

                # Disable
                set_bluez_property("Powered", "false")
                actual = stdoutRead(self.context_client,1)
                expected = ["Bluetooth.Enabled = bool:false"]
                self.assertEqual(actual, expected)

if __name__ == "__main__":
        sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)
        signal.signal(signal.SIGALRM, timeoutHandler)
        signal.alarm(30)
        unittest.main()
