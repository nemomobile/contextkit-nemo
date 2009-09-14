#!/usr/bin/env python
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

import re
import time
from subprocess import Popen, PIPE

class CLTool:
    STDOUT = 1
    STDERR = 2
    def __init__(self, *cline):
        self.__process = Popen(cline, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        self.__io = []

    def send(self, string):
        self.__io.append((0, string))
        print >>self.__process.stdin, string

    def expect(self, fileno, exp_str, timeout):
        stream = 0
        if fileno == self.STDOUT: stream = self.__process.stdout
        if fileno == self.STDERR: stream = self.__process.stderr
        if stream == 0: return False

        print >>self.__process.stdin, "flush"
        cur_str = ""
        start_time = time.time()
        while True:
            line = stream.readline().rstrip()
            if line == "FLUSHED":
                if re.match(exp_str, cur_str):
                    return True
                else:
                    time.sleep(0.1)
                    if time.time() - start_time > timeout:
                        self.printio()
                        print "Expected:", exp_str
                        print "Received before the timeout:\n", cur_str
                        return False
                    print >>self.__process.stdin, "flush"
            else:
                cur_str += line + "\n"
                self.__io.append((fileno, line))

    def printio(self):
        print
        print '----------------------------------------------------'
        for line in self.__io:
            if line[0] == 0:
                print "[IN] <<<", line[1]
            if line[0] == 1:
                print "[OU] >>>", line[1]
            if line[0] == 2:
                print "[ER] >>>", line[1]
        print '----------------------------------------------------'

    def wanted(name, type, value):
        return "%s = %s:%s$" % (name, type, value)
    wanted = staticmethod(wanted)

    def wantedUnknown(name):
        return "%s is Unknown" % (name)
    wantedUnknown = staticmethod(wantedUnknown)
