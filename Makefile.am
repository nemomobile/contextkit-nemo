SUBDIRS = contextd common libcontextsubscriber-plugins

ACLOCAL_AMFLAGS = -I m4

check-customer:
	$(MAKE) -C libcontextsubscriber-plugins/customer-tests check-customer

EXTRA_DIST = autoclean.sh autogen.sh

.PHONY: check-customer
