all-local: doxygen

DOXYCFG = $(srcdir)/doxy.cfg

doxygen:
	@if test x$(srcdir) = x. ; then \
		echo srcdir=$(srcdir) $(DOXYGEN) $(DOXYCFG); \
		srcdir=$(srcdir) $(DOXYGEN) $(DOXYCFG); \
	else \
		echo This is a VPATH build, so not building doxygen; \
	fi

EXTRA_DIST = $(DOXYCFG)

distclean-local:
	rm -f doxygen.log
	rm -rf html

.PHONY: doxygen
