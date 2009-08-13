check_PROGRAMS = lowmemproviderunittest

# test's sources
lowmemproviderunittest_SOURCES = lowmemproviderunittest.cpp property.h boolsysfspooler.h

COVERAGE_FILES = lowmemprovider.cpp 

# do the testing, coverage, etc. stuff
# tests.am is using +=, so we have to set a value here for these four always
AM_CXXFLAGS = $(QtCore_CFLAGS) $(QtDBus_CFLAGS)
AM_LDFLAGS = $(QtCore_LIBS) $(QtDBus_LIBS)
FROM_SOURCE = loggingfeatures.h lowmemprovider.h lowmemprovider.cpp # copy these files from the real source
FROM_SOURCE_DIR = $(srcdir)/../../src
LDADD = 
include $(top_srcdir)/am/tests.am

nodist_lowmemproviderunittest_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(lowmemproviderunittest_SOURCES) $(FROM_SOURCE))
include $(top_srcdir)/am/qt.am

# when MOCing from .cpp file, you also has to help the dependency tracker
lowmemproviderunittest.o: lowmemproviderunittest.moc
