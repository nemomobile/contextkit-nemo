SUBDIRS = contextd common libcontextsubscriber-plugins

ACLOCAL_AMFLAGS = -I m4

coverage: covoptioncheck
	$(MAKE) -C contextd coverage \
	&& $(MAKE) -C libcontextsubscriber-plugins coverage

check-customer:
	$(MAKE) -C libcontextsubscriber-plugins/customer-tests check-customer

EXTRA_DIST = autoclean.sh autogen.sh

.PHONY: check-customer

include $(top_srcdir)/am/covoptioncheck.am
