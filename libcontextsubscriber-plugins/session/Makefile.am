plugindir = $(libdir)/contextkit/subscriber-plugins/
plugin_LTLIBRARIES = session-1.la
session_1_la_SOURCES = sessionplugin.cpp sessionplugin.h loggingfeatures.h

AM_CXXFLAGS = -I$(top_srcdir)/common $(QtCore_CFLAGS) $(QtDBus_CFLAGS)	\
	      $(CTXTSUBS_CFLAGS) $(x11_CFLAGS)					\
	      '-DCONTEXT_LOG_MODULE_NAME="sessionstateplugin"'

# module: Libtool shouldn't complain about not having "lib" prefix
# avoid-version: Don't add version numbers after .so
session_1_la_LDFLAGS = -avoid-version -module

$(top_builddir)/common/libcommon.la:
	$(MAKE) -C $(top_builddir)/common libcommon.la

LIBS += $(QtCore_LIBS) $(x11_LIBS)
session_1_la_LIBADD=$(top_builddir)/common/libcommon.la

.PHONY: FORCE

# moccing
nodist_session_1_la_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(session_1_la_SOURCES))
include $(top_srcdir)/am/qt.am

# Distribute the .context file
providerdir = $(datadir)/contextkit/providers/

dist_provider_DATA = session.context
