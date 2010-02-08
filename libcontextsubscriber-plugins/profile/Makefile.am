plugindir = $(libdir)/contextkit/subscriber-plugins/
plugin_LTLIBRARIES = profile-1.la
profile_1_la_SOURCES = profileplugin.cpp profileplugin.h profile_dbus.h

AM_CXXFLAGS = -I$(top_srcdir)/common $(QtCore_CFLAGS) $(QtDBus_CFLAGS)	\
	      $(CTXTSUBS_CFLAGS)					\
	      '-DCONTEXT_LOG_MODULE_NAME="profileplugin"'

# module: Libtool shouldn't complain about not having "lib" prefix
# avoid-version: Don't add version numbers after .so
profile_1_la_LDFLAGS = -avoid-version -module

$(top_builddir)/common/libcommon.a:
	$(MAKE) -C $(top_builddir)/common libcommon.a

LIBS += $(QtCore_LIBS) $(QtDBus_LIBS)
profile_1_la_LIBADD=$(top_builddir)/common/libcommon.la

.PHONY: $(top_builddir)/common/libcommon.la

# moccing
nodist_profile_1_la_SOURCES = mocs.cpp
QT_TOMOC = $(filter %.h, $(profile_1_la_SOURCES))
include $(top_srcdir)/am/qt.am

# Distribute the .context file
providerdir = $(datadir)/contextkit/providers/

dist_provider_DATA = profile.context
