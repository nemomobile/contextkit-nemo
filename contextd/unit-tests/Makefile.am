SUBDIRS = halprovider boolsysfspooler lowmemprovider

check-sum: all
	@list='$(SUBDIRS)'; for subdir in $$list; do \
          (cd $$subdir && tput bold && echo $$subdir:\  && tput sgr0 && ( $(MAKE) $(AM_MAKEFLAGS) check | grep '^Total' | grep -v '^make' ) ) \
        done

coverage: covoptioncheck
	@list='$(SUBDIRS)'; for subdir in $$list; do \
          (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) coverage) \
        done
	mkdir -p coverage
	lcov $(patsubst %, -a %/coverage/selected.cov, $(SUBDIRS)) -o coverage/selecteds.cov
	@echo "real sources: `cd $(abs_top_srcdir) && pwd`"
	@echo "build: $(abs_top_builddir)"
	cat coverage/selecteds.cov | \
		sed "s,`cd $(abs_top_builddir) && pwd`,`cd $(abs_top_srcdir) && pwd`," | \
		sed 's,unit-tests/.*mock_,src/,' | \
		sed 's,unit-tests/.*src_,src/,' >coverage/sedded.cov
	genhtml --prefix `cd $(abs_top_srcdir) && pwd` -o coverage/ coverage/sedded.cov

clean-local:
	rm -rf coverage

.PHONY: coverage check-sum

include $(top_srcdir)/am/covoptioncheck.am
