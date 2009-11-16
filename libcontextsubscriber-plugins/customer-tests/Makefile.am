SUBDIRS = bluez fullscreen

coverage: covoptioncheck
	$(MAKE) -C bluez coverage \
	&& $(MAKE) -C fullscreen coverage

check-customer:
	$(MAKE) -C bluez check-customer \
	&& $(MAKE) -C fullscreen check-customer

EXTRA_DIST = waitforsilence

include $(top_srcdir)/am/covoptioncheck.am
