plugindir = $(libdir)/contextkit/subscriber-plugins/
plugin_LTLIBRARIES = libcontextsubscriberbluez.la
libcontextsubscriberbluez_la_SOURCES = bluezinterface.cpp	\
	bluezinterface.h bluezplugin.cpp bluezplugin.h		\
	loggingfeatures.h

AM_CXXFLAGS = -I$(top_srcdir)/common $(QtCore_CFLAGS) $(QtDBus_CFLAGS)	\
	      $(CTXTSUBS_CFLAGS)					\
	      '-DCONTEXT_LOG_MODULE_NAME="bluezplugin"'

$(top_builddir)/common/libcommon.a:
	$(MAKE) -C $(top_builddir)/common libcommon.a

LIBS += $(QtCore_LIBS) $(QtDBus_LIBS) 
libcontextsubscriberbluez_la_LIBADD=$(top_builddir)/common/libcommon.la

.PHONY: $(top_builddir)/common/libcommon.la

# moccing
nodist_libcontextsubscriberbluez_la_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(libcontextsubscriberbluez_la_SOURCES))
include $(top_srcdir)/am/qt.am

# Distribute the .context file
providerdir = $(datadir)/contextkit/providers/

dist_provider_DATA = bluez.context
