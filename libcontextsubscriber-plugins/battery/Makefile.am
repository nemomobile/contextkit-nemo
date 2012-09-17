
plugindir = $(libdir)/contextkit/subscriber-plugins/
plugin_LTLIBRARIES = battery-1.la
battery_1_la_SOURCES = batteryplugin.cpp batteryplugin.h

AM_CXXFLAGS = -I$(top_srcdir)/common $(QtCore_CFLAGS)			\
	      $(CTXTSUBS_CFLAGS) 					\
	      '-DCONTEXT_LOG_MODULE_NAME="batteryplugin"'		\
	      '-DCONTEXT_LOG_HIDE_DEBUG'

# module: Libtool shouldn't complain about not having "lib" prefix
# avoid-version: Don't add version numbers after .so
battery_1_la_LDFLAGS = -avoid-version -module

$(top_builddir)/common/libcommon.la: FORCE
	$(MAKE) -C $(top_builddir)/common libcommon.la

LIBS += $(QtCore_LIBS) -lbmeipc $(CTXTSUBS_LIBS)
battery_1_la_LIBADD=$(top_builddir)/common/libcommon.la

.PHONY: FORCE

# moccing
nodist_battery_1_la_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(battery_1_la_SOURCES))
include $(top_srcdir)/am/qt.am

# Distribute the .context file
providerdir = $(datadir)/contextkit/providers/

dist_provider_DATA = battery.context
