SUBDIRS = src

MOSTLYCLEANFILES= *~ *.bak config.log
MAINTAINERCLEANFILES= Makefile.in configure aclocal.m4 

# Copy all the spec files. Of course, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done
