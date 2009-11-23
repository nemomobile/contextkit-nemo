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
        self.provider = CLTool("context-provide", "--v2", "--session", "org.maemo.fake.provider", "bool", "Screen.Blanked", "false")
        self.context_client = CLTool("context-listen", "Session.State")

    def tearDown(self):
        self.context_client.close()
        self.provider.close()

    def testInitial(self):
        self.assert_(self.context_client.expect('Session.State = QString:"normal"'))

    def testBlanking(self):
        self.assert_(self.context_client.expect('Session.State = QString:"normal"'))

        self.provider.send("Screen.Blanked=true")
        self.assert_(self.context_client.expect('Session.State = QString:"blanked"'))

        self.provider.send("Screen.Blanked=false")
        self.assert_(self.context_client.expect('Session.State = QString:"normal"'))

        self.provider.send("Screen.Blanked=true")
        self.assert_(self.context_client.expect('Session.State = QString:"blanked"'))

        self.provider.send("unset Screen.Blanked")
        self.assert_(self.context_client.expect('Session.State = QString:"normal"'))

if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)
    signal.signal(signal.SIGALRM, timeoutHandler)
    signal.alarm(30)
    unittest.main()
