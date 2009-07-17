include ../Makefile.gtester

VALA_PACKAGES = --pkg dbus-glib-1 --pkg gee-1.0
VALA_OPTIONS = -g -C --basedir $(top_srcdir) --vapidir=$(top_srcdir)/vapi $(VALA_PACKAGES)

TEST_PROGS += test_hal_plugin

noinst_PROGRAMS = $(TEST_PROGS)

BUILT_SOURCES = test_hal_plugin.vala.stamp

# Real source files tested by this test
test_hal_plugin_real_VALASOURCES =		\
	$(top_srcdir)/contextd/hal_plugin.vala	\
	$(top_srcdir)/contextd/plugin.vala

test_hal_plugin_copied_VALASOURCES = hal_plugin.vala plugin.vala

# Files of the test program
# Note: libhal.vala and context_provider.vala are mock implementations
test_hal_plugin_test_VALASOURCES = test_hal_plugin.vala libhal.vala	\
	context_provider.vala

test_hal_plugin_VALASOURCES = $(test_hal_plugin_real_VALASOURCES)	\
	$(test_hal_plugin_test_VALASOURCES)

test_hal_plugin_SOURCES = $(test_hal_plugin_test_VALASOURCES:.vala=.c)	\
	$(test_hal_plugin_copied_VALASOURCES:.vala=.c)

if CONTEXTKIT_BUILD_VALA
test_hal_plugin.vala.stamp: $(top_srcdir)/vapi/posix.vapi $(test_hal_plugin_VALASOURCES)
	cp $(test_hal_plugin_real_VALASOURCES) .
	$(VALAC) $(VALA_OPTIONS) $(test_hal_plugin_test_VALASOURCES) $(test_hal_plugin_copied_VALASOURCES)
	touch $@
	rm $(test_hal_plugin_copied_VALASOURCES)
endif

test_hal_plugin_CFLAGS = $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GDBUS_CFLAGS) $(GEE_CFLAGS) $(HAL_CFLAGS)
test_hal_plugin_LDADD = $(GLIB_LIBS) $(GOBJECT_LIBS) $(GDBUS_LIBS) $(GEE_LIBS) $(HAL_LIBS) -lm

EXTRA_DIST += $(test_hal_plugin_test_VALASOURCES)		\
	$(test_hal_plugin_SOURCES) test_hal_plugin.vala.stamp

MAINTAINERCLEANFILES = $(test_hal_plugin_SOURCES)	\
	test_hal_plugin.vala.stamp
