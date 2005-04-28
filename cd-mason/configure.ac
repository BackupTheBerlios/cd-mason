AC_INIT([CD-Mason], [0.2], [hazzl@gmx.net])
AC_CONFIG_SRCDIR([src/cd-mason.svg])
AM_INIT_AUTOMAKE([dist-bzip2])

dnl Checks for programs.
AC_ISC_POSIX
AC_PROG_CC
dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([regex.h])
dnl Checks for libraries.
PKG_CHECK_MODULES(GTK, glib-2.0 >= 2.6.0 gdk-2.0 gtk+-2.0 >= 2.6.0 gnome-vfs-2.0,,exit)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
dnl Checks for typedefs, structures, and compiler characteristics.
dnl Checks for library functions.
AC_REPLACE_FUNCS(stpcpy)
dnl Checks for Additional stuffs.

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([ 
Makefile
src/Makefile
])
AC_OUTPUT
