#!/usr/bin/env python
##
## @file ft_memorypressure.py
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
from subprocess import Popen, PIPE
from time import sleep

class LowmemKernelModule(unittest.TestCase):
    """
    LowmemKernelModule Test
    """

    def setUp(self):
        """
		setUp
        """
        pass

    def tearDown(self):
        """
		tearDown
        """
        pass

    def test_watermarks_exists(self):
        """
		Test if Watermarks from lowmem kernel module exist
        """
        self.failIf(not os.path.exists("/sys/kernel/low_watermark"),
                    "/sys/kernel/low_watermark does not exist")
        self.failIf(not os.path.exists("/sys/kernel/high_watermark"),
                    "/sys/kernel/high_watermark does not exist")
        self.failIf(not os.path.isfile("/sys/kernel/low_watermark"),
                    "/sys/kernel/low_watermark is not a file")
        self.failIf(not os.path.isfile("/sys/kernel/high_watermark"),
                    "/sys/kernel/high_watermark is not a file")

class MemoryPressure(unittest.TestCase):
    """
	Memory Pressure
	"""

    def setUp(self):
        """
		setUp
        """
        self.memload = None
        self.memload2 = None
        self.contextd = Popen("contextd", stdin=PIPE, stderr=PIPE, stdout=PIPE)
        self.context_client = Popen(["context-listen", "System.MemoryPressure"],
                                    stdin=PIPE, stdout=PIPE, stderr=PIPE)
        cmd = 'echo 15000 > /proc/sys/vm/lowmem_notify_high_pages'
        os.system(cmd)

    def tearDown(self):
        """
		tearDown
        """
        os.kill(self.contextd.pid, 9)
        os.kill(self.context_client.pid, 9)

    def test_memory_ressure(self):
        """
        Description

        Pre-conditions

        Steps

        Post-conditions

        References

        """
        actual = self.context_client.stdout.readline().rstrip()
        expected = "System.MemoryPressure = int:0" # Normal
        self.assertEqual(actual, expected,
						 "Actual key value pairs do not match expected")

        cmd = 'memload 100MB'
        self.memload = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        sleep(2)

        actual = self.context_client.stdout.readline().rstrip()
        expected = "System.MemoryPressure = int:1" # High
        self.assertEqual(actual, expected,
						 "Actual key value pairs do not match expected")

        cmd = 'memload 150MB'
        self.memload2 = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        sleep(2)

        actual = self.context_client.stdout.readline().rstrip()
        expected = "System.MemoryPressure = int:2" # Critical
        self.assertEqual(actual,  expected,
						 "Actual key value pairs do not match expected")
        os.kill(self.memload.pid, 9)
        os.kill(self.memload2.pid, 9)

def run_tests():
    """
    runTests method
    """
    test_results = unittest.TestResult
    suite_lowmem_exists = unittest.TestLoader().loadTestsFromTestCase(LowmemKernelModule)
    test_results = unittest.TextTestRunner(verbosity=2).run(suite_lowmem_exists)
    if not test_results.wasSuccessful():
        sys.exit(1)

if __name__ == "__main__":
    run_tests()
