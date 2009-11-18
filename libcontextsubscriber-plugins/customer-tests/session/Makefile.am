SUBDIRS = screentoggler

coverage: covoptioncheck
	COVERAGE=1 ./runTests.sh
.PHONY: coverage

clean-local:
	-$(MAKE) -C coverage-build clean
	rm -rf coverage

check-customer:
	./runTests.sh

EXTRA_DIST = tests.xml runTests.sh test-fullscreen.py test-blanking.py coverage-build session.context fake-blanking.context

include $(top_srcdir)/am/covoptioncheck.am
