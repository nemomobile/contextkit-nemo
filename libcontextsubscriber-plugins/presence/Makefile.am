plugindir = $(libdir)/contextkit/subscriber-plugins/
plugin_LTLIBRARIES = presence-1.la
presence_1_la_SOURCES = presenceplugin.cpp presenceplugin.h loggingfeatures.h

# NOTE: using MT_CFLAGS shouldn't be needed; this is a workaround for
# a bug in the .pc file of presence0

AM_CXXFLAGS = -I$(top_srcdir)/common $(QtCore_CFLAGS) $(QtDBus_CFLAGS)	\
	      $(CTXTSUBS_CFLAGS) $(presence0_CFLAGS) $(MT_CFLAGS)
	      '-DCONTEXT_LOG_MODULE_NAME="presenceplugin"'

# module: Libtool shouldn't complain about not having "lib" prefix
# avoid-version: Don't add version numbers after .so
presence_1_la_LDFLAGS = -avoid-version -module

$(top_builddir)/common/libcommon.la:
	$(MAKE) -C $(top_builddir)/common libcommon.la

LIBS += $(QtCore_LIBS) $(presence0_LIBS) $(CTXTSUBS_LIBS)
presence_1_la_LIBADD=$(top_builddir)/common/libcommon.la

.PHONY: FORCE

# moccing
nodist_presence_1_la_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(presence_1_la_SOURCES))
include $(top_srcdir)/am/qt.am

# Distribute the .context file
providerdir = $(datadir)/contextkit/providers/

dist_provider_DATA = presence.context
