testdir = $(libdir)/libcontextsubscriberfullscreen-tests
test_PROGRAMS = screentoggler
screentoggler_SOURCES = main.cpp commandwatcher.h commandwatcher.cpp

AM_CXXFLAGS = $(QtCore_CFLAGS) $(QtGui_CFLAGS)
LIBS += $(QtCore_LIBS) $(QtGui_LIBS)

# moccing
nodist_screentoggler_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(screentoggler_SOURCES))
include $(top_srcdir)/am/qt.am
