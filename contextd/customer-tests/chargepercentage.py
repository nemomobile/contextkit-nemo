#!/usr/bin/env python
##
## @file ft_chargepercentage.py
## This file is part of ContextKit.
##
## Copyright (C) 2008, 2009 Nokia. All rights reserved.
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
import sys
import unittest
import os
import signal
from subprocess import Popen, PIPE
from time import sleep

def timeoutHandler(signum, frame):
    raise Exception('Tests have been running for too long')

class ChargePercentage(unittest.TestCase):
    """
    Charge percentage
    """

    def setUp(self):
        """
        setUp
        """
        self.contextd = Popen("contextd", stdin=PIPE, stderr=PIPE, stdout=PIPE)
        self.context_client = Popen(["context-listen", "Battery.ChargePercentage"],
                                    stdin=PIPE, stdout=PIPE, stderr=PIPE)

    def tearDown(self):
        """
        tearDown
        """
        os.kill(self.contextd.pid, 9)
        os.kill(self.context_client.pid, 9)

    def test_charge_percentage(self):
        """
        Description
        
        Tests whether contextd exposes the Battery.ChargePercentage property.

        Pre-conditions

        contextd and context-listen are running (see setUp).

        Steps

        Subscribe to the property with context-listen.

        Post-conditions

        The property has a value (it's not unknown). The value is greater than zero
        but smaller or equal to 100.

        References

        """
        actual = self.context_client.stdout.readline().rstrip()
        expectedStart = "Battery.ChargePercentage = int:"
        self.assert_(actual.startswith(expectedStart),
						 "Did not get a value for Battery.ChargePercentage")

        percentage = int(actual[len(expectedStart):])
        self.assert_(percentage > 0)
        self.assert_(percentage <= 100)

def runTests():
    suiteChargePercentage = unittest.TestLoader().loadTestsFromTestCase(ChargePercentage)
    result = unittest.TextTestRunner(verbosity=2).run(suiteChargePercentage)
    return len(result.errors + result.failures)

if __name__ == "__main__":
    signal.signal(signal.SIGALRM, timeoutHandler)
    signal.alarm(10)
    sys.exit(runTests())
