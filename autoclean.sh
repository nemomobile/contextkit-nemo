#! /bin/sh

# Remove the files that autogen.sh has created.

rm -f aclocal.m4 config.guess config.sub configure depcomp install-sh missing
rm -rf autom4te.cache
rm -f ltmain.sh m4/libtool.m4 m4/ltoptions.m4 m4/ltsugar.m4 m4/ltversion.m4 m4/lt~obsolete.m4
rm -f `find . -name Makefile.in`
