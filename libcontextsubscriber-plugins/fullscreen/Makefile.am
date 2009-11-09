plugindir = $(libdir)/contextkit/subscriber-plugins/
plugin_LTLIBRARIES = fullscreen-1.la
fullscreen_1_la_SOURCES = fullscreenplugin.cpp fullscreenplugin.h loggingfeatures.h

AM_CXXFLAGS = -I$(top_srcdir)/common $(QtCore_CFLAGS) $(QtDBus_CFLAGS)	\
	      $(CTXTSUBS_CFLAGS) $(x11_CFLAGS)					\
	      '-DCONTEXT_LOG_MODULE_NAME="fullscreenplugin"'

# module: Libtool shouldn't complain about not having "lib" prefix
# avoid-version: Don't add version numbers after .so
fullscreen_1_la_LDFLAGS = -avoid-version -module

$(top_builddir)/common/libcommon.a:
	$(MAKE) -C $(top_builddir)/common libcommon.a

LIBS += $(QtCore_LIBS) $(QtDBus_LIBS) $(x11_LIBS)
fullscreen_1_la_LIBADD=$(top_builddir)/common/libcommon.la

.PHONY: $(top_builddir)/common/libcommon.la

# moccing
nodist_fullscreen_1_la_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(fullscreen_1_la_SOURCES))
include $(top_srcdir)/am/qt.am

# Distribute the .context file
providerdir = $(datadir)/contextkit/providers/

dist_provider_DATA = fullscreen.context
