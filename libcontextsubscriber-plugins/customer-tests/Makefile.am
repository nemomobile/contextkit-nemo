SUBDIRS = bluez session dbusnamewatcher profile

coverage: covoptioncheck
	$(MAKE) -C bluez coverage \
	&& $(MAKE) -C profile coverage \
	&& $(MAKE) -C session coverage

check-customer:
	$(MAKE) -C bluez check-customer \
	&& $(MAKE) -C profile check-customer \
	&& $(MAKE) -C session check-customer

EXTRA_DIST = waitforsilence

include $(top_srcdir)/am/covoptioncheck.am
