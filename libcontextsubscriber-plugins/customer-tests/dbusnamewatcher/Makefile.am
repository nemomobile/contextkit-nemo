bin_PROGRAMS = dbusnamewatcher

dbusnamewatcher_SOURCES = dbusnamewatcher.cpp dbusnamewatcher.h

AM_CXXFLAGS = $(QtCore_CFLAGS) $(QtDBus_CFLAGS) \
	      '-DCONTEXT_LOG_MODULE_NAME="dbusnamewatcher"'
LIBS += $(QtCore_LIBS) $(QtDBus_LIBS)

# moccing
nodist_dbusnamewatcher_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(dbusnamewatcher_SOURCES))
include $(top_srcdir)/am/qt.am
