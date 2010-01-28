coverage: covoptioncheck
	COVERAGE=1 ./runTests.sh
.PHONY: coverage

clean-local:
	-$(MAKE) -C coverage-build clean
	rm -rf coverage

check-customer:
	./runTests.sh

EXTRA_DIST = tests.xml runTests.sh test-profile-plugin.py test-profile.context coverage-build

include $(top_srcdir)/am/covoptioncheck.am

