greeter_PROGRAMS = lightdm-tex-greeter
greeterdir = $(bindir)

lightdm_tex_greeter_SOURCES = \
	lightdm-tex-greeter.c

lightdm_tex_greeter_CFLAGS = \
	$(GREETER_CFLAGS) \
	$(WARN_CFLAGS) \
	-DVERSION=\"$(VERSION)\" \
	-DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
	-DLOCALE_DIR=\"$(localedir)\" \
	-DBINARY=\"lightdm-tex-greeter\"

lightdm_tex_greeter_LDADD = \
	$(GREETER_LIBS)

DISTCLEANFILES = \
	Makefile.in
