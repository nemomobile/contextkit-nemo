bin_PROGRAMS = battest
battest_SOURCES = battest.cpp battest.hpp

AM_CPPFLAGS =  $(QtCore_CFLAGS) $(CTXTSUBS_CFLAGS)
LIBS += $(QtCore_LIBS) $(CTXTSUBS_LIBS)

# moccing
nodist_battest_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.hpp, $(battest_SOURCES))
include $(top_srcdir)/am/qt.am
