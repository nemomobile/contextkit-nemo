SUBDIRS = src doc unit-tests customer-tests

providerdir = $(datadir)/contextkit/providers/
oldproviderdir = $(datadir)/context-providers/

dist_provider_DATA = org.maemo.contextd.context
dist_oldprovider_DATA = contextd.xml

servicedir = $(datadir)/dbus-1/services
service_DATA = org.maemo.contextd.service

check-sum:
	$(MAKE) -C unit-tests check-sum

coverage: covoptioncheck
	$(MAKE) -C unit-tests coverage

.PHONY: coverage check-sum

include $(top_srcdir)/am/covoptioncheck.am
