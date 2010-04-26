bin_PROGRAMS = contextd
contextd_SOURCES = contextd.cpp

AM_CXXFLAGS = $(CTXTPROV_CFLAGS) $(QtCore_CFLAGS) $(QtDBus_CFLAGS)
LIBS += $(CTXTPROV_LIBS) $(QtCore_LIBS)

# library dependency hack for seamless make in contextd
AM_CXXFLAGS += '-DCONTEXT_LOG_MODULE_NAME="contextd"'

# moccing
#nodist_contextd_SOURCES = mocs.cpp
#QT_TOMOC = $(filter %.h, $(contextd_SOURCES))
include $(top_srcdir)/am/qt.am
