if NEVER
bin_PROGRAMS = main
main_SOURCES = main.cpp batterywatcher.h

AM_CPPFLAGS =  $(QtCore_CFLAGS) -I/usr/include/bme
LIBS += $(QtCore_LIBS) -lbmeipc

# moccing
nodist_main_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(main_SOURCES))
include $(top_srcdir)/am/qt.am
endif
