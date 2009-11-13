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

def set_bluez_property(property, value):
    os.system("dbus-send --system --dest=org.bluez  --print-reply --type=method_call /org/bluez/`pidof bluetoothd`/hci0 org.bluez.Adapter.SetProperty string:%s variant:boolean:%s" % (property, value))
    time.sleep(1)


class BluezPlugin(unittest.TestCase):

    def setUp(self):
        os.environ["CONTEXT_PROVIDERS"] = "."
        # Make Bluetooth invisible and un-enabled
        # Note: This test will alter the bluetooth settings of the system!
        os.system("hciconfig -a")
        os.system("ps -Af | grep -i bluetooth")
        os.system("stop bluetoothd")
        os.system("start bluetoothd &")
        os.system("dbusnamewatcher --system org.bluez 10")
        os.system("qdbus --literal --system org.bluez / org.bluez.Manager.DefaultAdapter")
        os.system("hciconfig -a")
        os.system("ps -Af | grep -i bluetooth")
        print "Restart is over"
        set_bluez_property("Discoverable", "false")
        set_bluez_property("Powered", "false")

        self.context_client = CLTool("context-listen", "Bluetooth.Enabled", "Bluetooth.Visible")

    def tearDown(self):

        # Restore some default values for Bluez
        set_bluez_property("Discoverable", "false")
        set_bluez_property("Powered", "true")

    def testInitial(self):
        self.assert_(self.context_client.expect("Bluetooth.Visible = bool:false\nBluetooth.Enabled = bool:false"))


    def testEnabledAndVisible(self):
        # Enable
        set_bluez_property("Powered", "true")
        self.assert_(self.context_client.expect("Bluetooth.Visible = bool:false\nBluetooth.Enabled = bool:true"))

        # Set visible
        set_bluez_property("Discoverable", "true")
        self.assert_(self.context_client.expect("Bluetooth.Visible = bool:true"))

        # Set invisible
        set_bluez_property("Discoverable", "false")
        self.assert_(self.context_client.expect("Bluetooth.Visible = bool:false"))

        # Disable
        set_bluez_property("Powered", "false")
        self.assert_(self.context_client.expect("Bluetooth.Enabled = bool:false"))

if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)
    signal.signal(signal.SIGALRM, timeoutHandler)
    signal.alarm(60)
    unittest.main()
