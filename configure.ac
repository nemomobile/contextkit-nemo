AC_PREREQ([2.61])
AC_INIT([ContextKit-Maemo], [0.0~unreleased], [marius.vollmer@nokia.com], ContextKit-Maemo)

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2 tar-ustar 1.9])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_LIBTOOL

# vala compiler (>= 0.7.1)
AX_FEATURE_DISABLEABLE([vala], [BUILD_VALA], [disable building of vala sources])
AX_DISABLE_FEATURE_ON_PROG([Vala compiler], [BUILD_VALA], [VALAC], [valac], [0.7.1])
AM_CONDITIONAL(CONTEXTKIT_BUILD_VALA, test "$missing_deps_BUILD_VALA" = "")

# check for libraries
PKG_CHECK_MODULES_SUBST([CONTEXTPROVIDER], [contextprovider-1.0 >= 0.2.11])
PKG_CHECK_MODULES_SUBST([GLIB], [glib-2.0 >= 2.12.0])
PKG_CHECK_MODULES_SUBST([GOBJECT], [gobject-2.0 >= 2.12.0])
PKG_CHECK_MODULES_SUBST([GDBUS], [dbus-glib-1])
PKG_CHECK_MODULES_SUBST([GEE], [gee-1.0 >= 0.1.3])
PKG_CHECK_MODULES_SUBST([HAL], [hal])

# coverage tools
AX_FEATURE_DISABLEABLE([coverage], [COVERAGE], [disable unittests' coverage support])
AX_DISABLE_FEATURE_ON_PROG([GCov coverage recording tool], [COVERAGE], [GCOV], [gcov])
AX_DISABLE_FEATURE_ON_PROG([LCov coverage visualization tool], [COVERAGE], [LCOV], [lcov])
AM_CONDITIONAL([CONTEXTKIT_COVERAGE], [test "$missing_deps_COVERAGE" = ""])

AC_CONFIG_FILES([
        Makefile
        contextd/Makefile
        contextd/org.maemo.contextd.service
        tests/Makefile
        tests/contextd_tests/Makefile
        vapi/Makefile
])

AC_OUTPUT

AX_FEATURE_SUMMARIZE([BUILD_VALA],
                     [Building of vala sources is enabled],
                     [Vala sources won't be built])

AX_FEATURE_SUMMARIZE([COVERAGE],
                     [Unit testing coverage support is enabled],
                     [Coverage won't be supported during unit testing])
