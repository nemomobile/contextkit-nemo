#!/bin/sh

dokill() {
    for cpid in `cat /proc/[0-9]*/stat | cut -d\  -f1,4 | grep " $1" | cut -d' ' -f1`
    do
        dokill $cpid
    done
    kill -9 $1 > /dev/null 2>&1
}

if test $# = 0
then
    echo "usage: $0 <top pid to kill>"
    exit 1
fi

for pid in $*
do
    dokill $pid
done
