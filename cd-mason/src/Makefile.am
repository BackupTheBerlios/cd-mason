## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = $(GTK_CFLAGS)
AM_CFLAGS = -Wall
bin_PROGRAMS = cd-mason
cd_mason_SOURCES = \
	main.c\
	main.h\
	main_window.c\
	main_window.h\
	menus.c\
	menus.h\
	tree_view.c\
	tree_view.h\
	cdrecord_iface.c\
	cdrecord_iface.h\
	preferences.c\
	preferences.h\
	progress_dialog.c\
	progress_dialog.h\
	options_window.c\
	options_window.h\
	project.h
EXTRA_DIST = \
	cd-mason.svg\
	clear-cdrw.png\
	stpcpy.c\
	type_data.png\
	type_audio.png\
	type_iso.png
	
cd_mason_LDADD = $(GTK_LIBS)
MAINTAINERCLEANFILES = Makefile.in config.h.in
MOSTLYCLEANFILES = *~ *.s
CLEANFILES = stamp-h1 stamp-h.in
.c.s:
	$(CC) $(AM_CPPFLAGS) -DHAVE_CONFIG_H -Os -fomit-frame-pointer -S $<
