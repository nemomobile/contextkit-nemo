check_PROGRAMS = boolsysfspoolerunittest

# test's sources
boolsysfspoolerunittest_SOURCES = boolsysfspoolerunittest.cpp

COVERAGE_FILES = boolsysfspooler.cpp

# do the testing, coverage, etc. stuff
# tests.am is using +=, so we have to set a value here for these four always
AM_CXXFLAGS = $(QtCore_CFLAGS) $(QtDBus_CFLAGS)
 
AM_LDFLAGS = $(QtCore_LIBS) $(QtDBus_LIBS)
FROM_SOURCE = loggingfeatures.h boolsysfspooler.cpp boolsysfspooler.h
FROM_SOURCE_DIR = $(srcdir)/../../src
LDADD = 
include $(top_srcdir)/am/tests.am

EXTRA_DIST=input1.txt	\
	   input2.txt	\
	   input3.txt

nodist_boolsysfspoolerunittest_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(boolsysfspoolerunittest_SOURCES) $(FROM_SOURCE))
include $(top_srcdir)/am/qt.am

# when MOCing from .cpp file, you also has to help the dependency tracker
boolsysfspoolerunittest.o: boolsysfspoolerunittest.moc
