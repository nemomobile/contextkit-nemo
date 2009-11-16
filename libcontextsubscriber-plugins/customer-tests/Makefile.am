SUBDIRS = bluez-plugin fullscreen-plugin

coverage: covoptioncheck
	$(MAKE) -C fullscreen-plugin coverage

check-customer:
	$(MAKE) -C bluez-plugin check-customer && $(MAKE) -C fullscreen-plugin check-customer

EXTRA_DIST = waitforsilence

include $(top_srcdir)/am/covoptioncheck.am
