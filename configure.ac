dnl Process this file with autoconf to produce a configure script.

AC_INIT(lightdm-tex-greeter, 0.1.0)
AC_SUBST(THEME_DIR)
AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])

dnl GNOME_COMPILE_WARNINGS(maximum)

dnl ###########################################################################
dnl Dependencies
dnl ###########################################################################

PKG_CHECK_MODULES(GREETER, [
    liblightdm-gobject-1
    gtk+-2.0
    webkit-1.0
    dbus-glib-1
])

dnl ###########################################################################
dnl Configurable values
dnl ###########################################################################
THEME_DIR="/usr/share/lightdm-tex/themes/"
AC_ARG_WITH(theme-dir,
            AS_HELP_STRING(--with-theme-dir=<path>,
                           Theme directory to use for tex-greeter themes),
    if test x$withval != x; then
        THEME_DIR="$withval"
    fi
)
AC_SUBST(THEME_DIR)
AC_DEFINE_UNQUOTED(THEME_DIR, "$THEME_DIR", Theme Dir)

CONFIG_DIR="/etc/lightdm/"
AC_ARG_WITH(config-dir,
            AS_HELP_STRING(--with-config-dir=<path>,
                           Configuration directory of lightdm),
    if test x$withval != x; then
        CONFIG_DIR="$withval"
    fi
)
AC_SUBST(CONFIG_DIR)
AC_DEFINE_UNQUOTED(CONFIG_DIR, "$CONFIG_DIR", Config Dir)

DESKTOP_DIR="/usr/share/xgreeters/"
AC_ARG_WITH(desktop-dir,
            AS_HELP_STRING(--with-desktop-dir=<path>,
                           Desktop directory of lightdm),
    if test x$withval != x; then
        DESKTOP_DIR="$withval"
    fi
)
AC_SUBST(DESKTOP_DIR)
AC_DEFINE_UNQUOTED(DESKTOP_DIR, "$DESKTOP_DIR", Desktop Dir)

dnl ###########################################################################
dnl Internationalization
dnl note that GETTEXT_PACKAGE has been defined in config.h.in.
dnl ###########################################################################

IT_PROG_INTLTOOL(0.35.0)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", Gettext package)

dnl ###########################################################################
dnl Files to generate
dnl ###########################################################################

AC_OUTPUT([
Makefile
data/Makefile
src/Makefile
po/Makefile.in
themes/Makefile
themes/webkitsimple/Makefile
])

dnl ###########################################################################
dnl Summary
dnl ###########################################################################

echo "
                    Light Display Manager Tex Greeter $VERSION
                    ==========================================

        prefix:                   $prefix
        Theme directory:          $THEME_DIR
        Configuration directory:  $CONFIG_DIR
"
