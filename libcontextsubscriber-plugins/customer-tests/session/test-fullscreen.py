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
import signal
from ContextKit.cltool import CLTool

def timeoutHandler(signum, frame):
    raise Exception('Tests have been running for too long')

class SessionPlugin(unittest.TestCase):

    def setUp(self):
        if "DISPLAY" not in os.environ:
            os.environ["DISPLAY"] = ":0.0"
        # start context-listen, but don't subscribe yet to anything
        # (to separate the start-up delay from the subscription +
        # initial value computation delay)
        self.context_client = CLTool("context-listen")
        # wait for the context-listen to start
        self.context_client.expect("Available commands")

    def tearDown(self):
        self.context_client.close()
        self.context_client.wait()

    def testProvider(self):
        self.context_client.send("new Session.State")
        self.context_client.send("providers Session.State")
        self.assert_(self.context_client.expect("providers: Session.State@/session-1\n"))

    def testInitial(self):
        self.context_client.send("new Session.State")
        self.assert_(self.context_client.expect('Session.State = QString:"normal"', timeout=10))

    def testStartFull(self):
        self.context_client.send("new Session.State")
        self.assert_(self.context_client.expect('Session.State = QString:"normal"', timeout=10))

        program = CLTool("screentoggler", "full")
        self.assert_(program.expect("ready"))

        self.assert_(self.context_client.expect('Session.State = QString:"fullscreen"', timeout=10))

        program.close()
        self.assert_(self.context_client.expect('Session.State = QString:"normal"', timeout=10))

    def testStartNormal(self):
        self.context_client.send("new Session.State")
        self.assert_(self.context_client.expect('Session.State = QString:"normal"', timeout=10))

        program = CLTool("screentoggler")
        self.assert_(program.expect("ready"))

        # no change signals should be delivered, so we need to query the value
        self.context_client.send("v Session.State")
        self.assert_(self.context_client.expect('value: QString:"normal"'))

        program.close()
        program.wait()
        self.context_client.send("v Session.State")
        self.assert_(self.context_client.expect('value: QString:"normal"'))

    def testToggleFullAndNormal(self):
        program = CLTool("screentoggler")
        self.assert_(program.expect("ready"))

        self.context_client.send("new Session.State")
        self.assert_(self.context_client.expect('Session.State = QString:"normal"', timeout=10))

        program.send("full")
        # wait for the change to happen and the helper program to print out "full"
        self.assert_(program.expect("full"))

        self.assert_(self.context_client.expect('Session.State = QString:"fullscreen"'))

        program.send("normal")
        self.assert_(program.expect("normal"))

        self.assert_(self.context_client.expect('Session.State = QString:"normal"'))

        program.close()
        program.wait()


if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)
    signal.signal(signal.SIGALRM, timeoutHandler)
    signal.alarm(30)
    unittest.main()
