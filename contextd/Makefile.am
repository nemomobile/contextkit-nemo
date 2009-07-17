CFLAGS = -Wall

bin_PROGRAMS = contextd

providerdir = $(datadir)/contextkit/providers/
oldproviderdir = $(datadir)/context-providers/

dist_provider_DATA = org.maemo.contextd.context
dist_oldprovider_DATA = contextd.xml

servicedir = $(datadir)/dbus-1/services
dist_service_DATA = org.maemo.contextd.service

contextd_VALASOURCES = main.vala plugin.vala hal_plugin.vala lowmem_plugin.vala

contextd_GENERATEDSOURCES = $(contextd_VALASOURCES:.vala=.c)
contextd_SOURCES = $(contextd_GENERATEDSOURCES)

if CONTEXTKIT_BUILD_VALA
# FIXME: Remove the hal.vapi once the official version includes the needed functions
contextd.vala.stamp: $(top_srcdir)/vapi/hal.vapi $(top_srcdir)/vapi/posix.vapi $(contextd_VALASOURCES)
	$(VALAC) -C -g --basedir $(top_srcdir)/contextd --vapidir=$(top_srcdir)/vapi --pkg contextprovider-1.0 --pkg dbus-glib-1 --pkg gee-1.0 --disable-dbus-transformation $^
	touch contextd.vala.stamp
endif

AM_CFLAGS = $(CONTEXTPROVIDER_CFLAGS) $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GDBUS_CFLAGS) $(GEE_CFLAGS) $(HAL_CFLAGS)

LDADD = $(CONTEXTPROVIDER_LIBS) $(GLIB_LIBS) $(GOBJECT_LIBS) $(GDBUS_LIBS) $(GEE_LIBS) $(HAL_LIBS)

EXTRA_DIST = $(contextd_GENERATEDSOURCES) $(contextd_VALASOURCES) \
	     contextd.vala.stamp

DISTCLEANFILES = *.gcno *.gcda

MAINTAINERCLEANFILES = $(contextd_GENERATEDSOURCES)	\
		       contextd.vala.stamp

BUILT_SOURCES = contextd.vala.stamp

install-data-hook:
	-update-context-providers $(DESTDIR)$(oldproviderdir)
	-update-contextkit-providers $(DESTDIR)$(providerdir)

uninstall-hook:
	-update-context-providers $(DESTDIR)$(oldproviderdir)
	-update-contextkit-providers $(DESTDIR)$(providerdir)
	[ 0 -lt `find $(DESTDIR)$(oldproviderdir) -name '*.xml' | wc -l` ] || rm -f $(DESTDIR)$(oldproviderdir)/context-providers.cdb
	[ 0 -lt `find $(DESTDIR)$(providerdir) -name '*.context' | wc -l` ] || rm -f $(DESTDIR)$(providerdir)/cache.cdb
