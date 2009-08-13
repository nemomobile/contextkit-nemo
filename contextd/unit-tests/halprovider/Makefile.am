check_PROGRAMS = halproviderunittest

# test's sources
halproviderunittest_SOURCES = halproviderunittest.cpp property.h haldeviceinterface.h halmanagerinterface.h group.h

COVERAGE_FILES = halprovider.cpp 

# do the testing, coverage, etc. stuff
# tests.am is using +=, so we have to set a value here for these four always
AM_CXXFLAGS = $(QtCore_CFLAGS) $(QtDBus_CFLAGS)
AM_LDFLAGS = $(QtCore_LIBS) $(QtDBus_LIBS)
FROM_SOURCE = loggingfeatures.h halprovider.h halprovider.cpp # copy these files from the real source
FROM_SOURCE_DIR = $(srcdir)/../../src
LDADD = 
include $(top_srcdir)/am/tests.am

nodist_halproviderunittest_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(halproviderunittest_SOURCES) $(FROM_SOURCE))
include $(top_srcdir)/am/qt.am

# when MOCing from .cpp file, you also has to help the dependency tracker
halproviderunittest.o: halproviderunittest.moc
