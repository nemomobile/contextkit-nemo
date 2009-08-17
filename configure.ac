AC_PREREQ([2.61])
AC_INIT([ContextKit-Maemo], [0], [marius.vollmer@nokia.com], ContextKit-Maemo)

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2 tar-ustar 1.9])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
CFLAGS="$CXXFLAGS -Wall"
AC_PROG_CXX
CXXFLAGS="$CXXFLAGS -Wall"
AC_PROG_LIBTOOL
AM_PATH_PYTHON

# check for libraries
PKG_CHECK_MODULES_SUBST([DBUS], [dbus-1])
PKG_CHECK_MODULES_SUBST([QtTest], [QtTest])
PKG_CHECK_MODULES_SUBST([QtCore], [QtCore])
PKG_CHECK_MODULES_SUBST([QtDBus], [QtDBus])
PKG_CHECK_MODULES_SUBST([QtXml], [QtXml])
PKG_CHECK_MODULES_SUBST([CTXTPROV], [contextprovider-1.0])

AC_PROG_QT_MOC
AC_PROG_QT_UIC
AC_PROG_QT_RCC
AC_CHECK_LIB([cdb], [cdb_make_start], [CDB_LIBS=-lcdb], [AC_MSG_ERROR([Can't find cdb library])])
AC_SUBST([CDB_LIBS])

# tools for documentation
AX_FEATURE_DISABLEABLE([doc], [BUILD_DOCS], [disable building of documentation])
AC_MSG_CHECKING([for gtk-doc])
PKG_CHECK_EXISTS([gtk-doc >= 1.9],[AC_MSG_RESULT([yes])],
			  [AC_MSG_RESULT([no])
			   missing_deps_BUILD_DOCS="$missing_deps_BUILD_DOCS, gtk-doc >= 1.9"])
HTML_DIR="${datadir}/gtk-doc/html"
AC_SUBST([HTML_DIR])
AX_DISABLE_FEATURE_ON_PROG([Dot drawing tool], [BUILD_DOCS], [DOT], [dot])
AX_DISABLE_FEATURE_ON_PROG([Asciidoc], [BUILD_DOCS], [ASCIIDOC], [asciidoc], [8.2.7])
AX_DISABLE_FEATURE_ON_PROG([source-highlight], [BUILD_DOCS], [SOURCE_HIGHLIGHT], [source-highlight])
AX_DISABLE_FEATURE_ON_PROG([xsltproc], [BUILD_DOCS], [XSLTPROC], [xsltproc])
AX_DISABLE_FEATURE_ON_PROG([xmllint], [BUILD_DOCS], [XMLLINT], [xmllint])
AX_DISABLE_FEATURE_ON_PROG([doxygen], [BUILD_DOCS], [DOXYGEN], [doxygen])
AM_CONDITIONAL([CONTEXTKIT_BUILD_DOCS], [test "$missing_deps_BUILD_DOCS" = ""])
AM_CONDITIONAL([GTK_DOC_USE_LIBTOOL], [test "$missing_deps_BUILD_DOCS" = ""])
AM_CONDITIONAL([ENABLE_GTK_DOC], [test "$missing_deps_BUILD_DOCS" = ""])

# coverage tools
AX_FEATURE_DISABLEABLE([coverage], [COVERAGE], [disable unittests' coverage support])
AX_DISABLE_FEATURE_ON_PROG([GCov coverage recording tool], [COVERAGE], [GCOV], [gcov])
AX_DISABLE_FEATURE_ON_PROG([LCov coverage visualization tool], [COVERAGE], [LCOV], [lcov])
AM_CONDITIONAL([CONTEXTKIT_COVERAGE], [test "$missing_deps_COVERAGE" = ""])

AC_CONFIG_FILES([
	Makefile
	contextd/Makefile
	contextd/org.maemo.contextd.service
	contextd/src/Makefile
	contextd/customer-tests/Makefile
	contextd/unit-tests/Makefile
	contextd/unit-tests/halprovider/Makefile
	contextd/unit-tests/boolsysfspooler/Makefile
	contextd/unit-tests/lowmemprovider/Makefile
	contextd/doc/Makefile
])

AC_OUTPUT

AX_FEATURE_SUMMARIZE([BUILD_DOCS],
		     [Building of documentation is enabled],
		     [Documentation won't be built])

AX_FEATURE_SUMMARIZE([COVERAGE],
		     [Unit testing coverage support is enabled],
		     [Coverage won't be supported during unit testing])
