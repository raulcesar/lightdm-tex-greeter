/*
 * Copyright (C) 2010 Robert Ancell.
 * Copyright (C) 2014 Raul Cesar Teixeira
 *
 *
 * Author: Raul Cesar Teixeira
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>
#include <glib/gi18n.h>

#include <lightdm.h>

#include <../config.h>

static JSClassRef gettext_class, lightdm_greeter_class, lightdm_user_class, lightdm_language_class, lightdm_layout_class, lightdm_session_class;

static WebKitWebView *web_view;
static GtkWidget *window;
static gchar *theme;

static void
show_prompt_cb (LightDMGreeter *greeter, const gchar *text, WebKitWebView *view)
{
    gchar *command;

    g_debug("Show prompt %s", text);

    command = g_strdup_printf ("show_prompt('%s')", text);
    webkit_web_view_execute_script (web_view, command);
    g_free (command);
}

static void
show_message_cb (LightDMGreeter *greeter, const gchar *text, WebKitWebView *view)
{
    gchar *command;

    command = g_strdup_printf ("show_message('%s')", text);
    webkit_web_view_execute_script (view, command);
    g_free (command);
}

static void
authentication_complete_cb (LightDMGreeter *greeter, WebKitWebView *view)
{
    webkit_web_view_execute_script (view, "authentication_complete()");
}

static void
autologin_timeout_expired_cb (LightDMGreeter *greeter, WebKitWebView *view)
{
    gchar *command = g_strdup_printf ("autologin_timeout_expired()");
    webkit_web_view_execute_script (view, command);
    g_free (command);
}

static gboolean
fade_timer_cb (gpointer data)
{
    gdouble opacity;

    opacity = gtk_window_get_opacity (GTK_WINDOW (window));
    opacity -= 0.1;
    if (opacity <= 0)
    {
        gtk_main_quit ();
        return FALSE;
    }
    gtk_window_set_opacity (GTK_WINDOW (window), opacity);

    return TRUE;
}

static void
quit_cb (LightDMGreeter *greeter, const gchar *username)
{
    /* Fade out the greeter */
    g_timeout_add (40, (GSourceFunc) fade_timer_cb, NULL);
}

static JSValueRef
get_user_name_cb (JSContextRef context,
                  JSObjectRef thisObject,
                  JSStringRef propertyName,
                  JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_user_get_name (user));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_user_real_name_cb (JSContextRef context,
                       JSObjectRef thisObject,
                       JSStringRef propertyName,
                       JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_user_get_real_name (user));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_user_display_name_cb (JSContextRef context,
                          JSObjectRef thisObject,
                          JSStringRef propertyName,
                          JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_user_get_display_name (user));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_user_image_cb (JSContextRef context,
                   JSObjectRef thisObject,
                   JSStringRef propertyName,
                   JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_user_get_image (user));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_user_language_cb (JSContextRef context,
                      JSObjectRef thisObject,
                      JSStringRef propertyName,
                      JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    const gchar *language = lightdm_user_get_language (user);
    JSStringRef string;

    if (!language)
        return JSValueMakeNull (context);

    string = JSStringCreateWithUTF8CString (language);
    return JSValueMakeString (context, string);
}

static JSValueRef
get_user_layout_cb (JSContextRef context,
                    JSObjectRef thisObject,
                    JSStringRef propertyName,
                    JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    const gchar *layout = lightdm_user_get_layout (user);
    JSStringRef string;

    if (!layout)
        return JSValueMakeNull (context);

    string = JSStringCreateWithUTF8CString (layout);
    return JSValueMakeString (context, string);
}

static JSValueRef
get_user_session_cb (JSContextRef context,
                     JSObjectRef thisObject,
                     JSStringRef propertyName,
                     JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    const gchar *session = lightdm_user_get_session (user);
    JSStringRef string;

    if (!session)
        return JSValueMakeNull (context);

    string = JSStringCreateWithUTF8CString (session);
    return JSValueMakeString (context, string);
}

static JSValueRef
get_user_logged_in_cb (JSContextRef context,
                       JSObjectRef thisObject,
                       JSStringRef propertyName,
                       JSValueRef *exception)
{
    LightDMUser *user = JSObjectGetPrivate (thisObject);
    return JSValueMakeBoolean (context, lightdm_user_get_logged_in (user));
}

static JSValueRef
get_language_code_cb (JSContextRef context,
                      JSObjectRef thisObject,
                      JSStringRef propertyName,
                      JSValueRef *exception)
{
    LightDMLanguage *language = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_language_get_code (language));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_language_name_cb (JSContextRef context,
                      JSObjectRef thisObject,
                      JSStringRef propertyName,
                      JSValueRef *exception)
{
    LightDMLanguage *language = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_language_get_name (language));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_language_territory_cb (JSContextRef context,
                           JSObjectRef thisObject,
                           JSStringRef propertyName,
                           JSValueRef *exception)
{
    LightDMLanguage *language = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_language_get_territory (language));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_layout_name_cb (JSContextRef context,
                    JSObjectRef thisObject,
                    JSStringRef propertyName,
                    JSValueRef *exception)
{
    LightDMLayout *layout = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_layout_get_name (layout));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_layout_short_description_cb (JSContextRef context,
                                 JSObjectRef thisObject,
                                 JSStringRef propertyName,
                                 JSValueRef *exception)
{
    LightDMLayout *layout = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_layout_get_short_description (layout));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_layout_description_cb (JSContextRef context,
                           JSObjectRef thisObject,
                           JSStringRef propertyName,
                           JSValueRef *exception)
{
    LightDMLayout *layout = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_layout_get_description (layout));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_session_key_cb (JSContextRef context,
                    JSObjectRef thisObject,
                    JSStringRef propertyName,
                    JSValueRef *exception)
{
    LightDMSession *session = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_session_get_key (session));
    return JSValueMakeString (context, string);

}
static JSValueRef
get_session_name_cb (JSContextRef context,
                     JSObjectRef thisObject,
                     JSStringRef propertyName,
                     JSValueRef *exception)
{
    LightDMSession *session = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_session_get_name (session));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_session_comment_cb (JSContextRef context,
                        JSObjectRef thisObject,
                        JSStringRef propertyName,
                        JSValueRef *exception)
{
    LightDMSession *session = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_session_get_comment (session));
    return JSValueMakeString (context, string);
}

static JSValueRef
get_hostname_cb (JSContextRef context,
                 JSObjectRef thisObject,
                 JSStringRef propertyName,
                 JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_get_hostname ());

    return JSValueMakeString (context, string);
}

static JSValueRef
get_num_users_cb (JSContextRef context,
                  JSObjectRef thisObject,
                  JSStringRef propertyName,
                  JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    gint num_users;

    num_users = g_list_length(lightdm_user_list_get_users (lightdm_user_list_get_instance()));
    return JSValueMakeNumber (context, num_users);
}

static JSValueRef
get_users_cb (JSContextRef context,
              JSObjectRef thisObject,
              JSStringRef propertyName,
              JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSObjectRef array;
    const GList *users, *link;
    guint i, n_users = 0;
    JSValueRef *args;

    users = lightdm_user_list_get_users( lightdm_user_list_get_instance() );
    n_users = g_list_length ((GList *)users);
    args = g_malloc (sizeof (JSValueRef) * (n_users + 1));
    for (i = 0, link = users; link; i++, link = link->next)
    {
        LightDMUser *user = link->data;
        g_object_ref (user);
        args[i] = JSObjectMake (context, lightdm_user_class, user);
    }

    array = JSObjectMakeArray (context, n_users, args, NULL);
    g_free (args);
    return array;
}

static JSValueRef
get_languages_cb (JSContextRef context,
                  JSObjectRef thisObject,
                  JSStringRef propertyName,
                  JSValueRef *exception)
{
    JSObjectRef array;
    const GList *languages, *link;
    guint i, n_languages = 0;
    JSValueRef *args;

    languages = lightdm_get_languages ();
    n_languages = g_list_length ((GList *)languages);
    args = g_malloc (sizeof (JSValueRef) * (n_languages + 1));
    for (i = 0, link = languages; link; i++, link = link->next)
    {
        LightDMLanguage *language = link->data;
        g_object_ref (language);
        args[i] = JSObjectMake (context, lightdm_language_class, language);
    }

    array = JSObjectMakeArray (context, n_languages, args, NULL);
    g_free (args);
    return array;
}

static JSValueRef
get_default_language_cb (JSContextRef context,
                         JSObjectRef thisObject,
                         JSStringRef propertyName,
                         JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_language_get_name((LightDMLanguage *)lightdm_get_language ()));

    return JSValueMakeString (context, string);
}

static JSValueRef
get_default_layout_cb (JSContextRef context,
                       JSObjectRef thisObject,
                       JSStringRef propertyName,
                       JSValueRef *exception)
{
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_layout_get_name(lightdm_get_layout ()));

    return JSValueMakeString (context, string);
}

static JSValueRef
get_layouts_cb (JSContextRef context,
                JSObjectRef thisObject,
                JSStringRef propertyName,
                JSValueRef *exception)
{
    JSObjectRef array;
    const GList *layouts, *link;
    guint i, n_layouts = 0;
    JSValueRef *args;

    layouts = lightdm_get_layouts ();
    n_layouts = g_list_length ((GList *)layouts);
    args = g_malloc (sizeof (JSValueRef) * (n_layouts + 1));
    for (i = 0, link = layouts; link; i++, link = link->next)
    {
        LightDMLayout *layout = link->data;
        g_object_ref (layout);
        args[i] = JSObjectMake (context, lightdm_layout_class, layout);
    }

    array = JSObjectMakeArray (context, n_layouts, args, NULL);
    g_free (args);
    return array;
}

static JSValueRef
get_layout_cb (JSContextRef context,
               JSObjectRef thisObject,
               JSStringRef propertyName,
               JSValueRef *exception)
{
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_layout_get_name(lightdm_get_layout ()));

    return JSValueMakeString (context, string);
}

static bool
set_layout_cb (JSContextRef context,
               JSObjectRef thisObject,
               JSStringRef propertyName,
               JSValueRef value,
               JSValueRef *exception)
{
    JSStringRef layout_arg;
    char layout[1024];

    // FIXME: Throw exception
    if (JSValueGetType (context, value) != kJSTypeString)
        return false;

    layout_arg = JSValueToStringCopy (context, value, NULL);
    JSStringGetUTF8CString (layout_arg, layout, 1024);
    JSStringRelease (layout_arg);

    //lightdm_set_layout (layout);

    return true;
}

static JSValueRef
get_sessions_cb (JSContextRef context,
                 JSObjectRef thisObject,
                 JSStringRef propertyName,
                 JSValueRef *exception)
{
    JSObjectRef array;
    const GList *sessions, *link;
    guint i, n_sessions = 0;
    JSValueRef *args;

    sessions = lightdm_get_sessions ();
    n_sessions = g_list_length ((GList *)sessions);
    args = g_malloc (sizeof (JSValueRef) * (n_sessions + 1));
    for (i = 0, link = sessions; link; i++, link = link->next)
    {
        LightDMSession *session = link->data;
        g_object_ref (session);
        args[i] = JSObjectMake (context, lightdm_session_class, session);
    }

    array = JSObjectMakeArray (context, n_sessions, args, NULL);
    g_free (args);
    return array;
}

static JSValueRef
get_default_session_cb (JSContextRef context,
                        JSObjectRef thisObject,
                        JSStringRef propertyName,
                        JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_greeter_get_default_session_hint (greeter));

    return JSValueMakeString (context, string);
}

static JSValueRef
get_timed_login_user_cb (JSContextRef context,
                         JSObjectRef thisObject,
                         JSStringRef propertyName,
                         JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef string;

    string = JSStringCreateWithUTF8CString (lightdm_greeter_get_autologin_user_hint (greeter));

    return JSValueMakeString (context, string);
}

static JSValueRef
get_timed_login_delay_cb (JSContextRef context,
                          JSObjectRef thisObject,
                          JSStringRef propertyName,
                          JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    gint delay;

    delay = lightdm_greeter_get_autologin_timeout_hint (greeter);
    return JSValueMakeNumber (context, delay);
}
static JSValueRef
cancel_timed_login_cb (JSContextRef context,
                       JSObjectRef function,
                       JSObjectRef thisObject,
                       size_t argumentCount,
                       const JSValueRef arguments[],
                       JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);

    // FIXME: Throw exception
    if (argumentCount != 0)
        return JSValueMakeNull (context);

    lightdm_greeter_cancel_autologin (greeter);
    return JSValueMakeNull (context);
}

static JSValueRef
start_authentication_cb (JSContextRef context,
                         JSObjectRef function,
                         JSObjectRef thisObject,
                         size_t argumentCount,
                         const JSValueRef arguments[],
                         JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef name_arg;
    char name[1024];

    // FIXME: Throw exception
    if (!(argumentCount == 1 && JSValueGetType (context, arguments[0]) == kJSTypeString))
        return JSValueMakeNull (context);

    name_arg = JSValueToStringCopy (context, arguments[0], NULL);
    JSStringGetUTF8CString (name_arg, name, 1024);
    JSStringRelease (name_arg);

    lightdm_greeter_authenticate (greeter, name);
    return JSValueMakeNull (context);
}

static gchar *
toGChar(JSStringRef jsstr)
{
  size_t size;
  gchar *buf;
  size = JSStringGetMaximumUTF8CStringSize(jsstr);
  buf = g_malloc0(size);
  JSStringGetUTF8CString(jsstr, buf, size);


  return buf;
}

static JSValueRef
getJSValueRefFromPropFile(JSContextRef context,
                           gchar *gUsr,
                           gchar *gProperty,
                           gchar* userConfFilename
                           )
{
    //Later we will get this from file.
    //GKeyFile *keyfile;
    GKeyFile *keyfile;
    GError *err = NULL;
    keyfile = g_key_file_new ();

    gboolean fileFound = g_key_file_load_from_file (keyfile, userConfFilename, G_KEY_FILE_NONE, &err);

    if (fileFound == FALSE) {
      g_message("Error attempting g_key_file_load_from_file: %s", err->message);
      g_error_free (err);
      return JSValueMakeNull (context);
    }

    //Query the keyfile for property.

    gchar *gResult = g_key_file_get_string(keyfile, gUsr, gProperty, &err);
    if (gResult == NULL) {
      g_message("Error attempting g_key_file_get_string: %s", err->message);
      g_error_free (err);
      return JSValueMakeNull (context);
    }

    //Free keyfile
    g_key_file_free(keyfile);

    //Free keyfile
    JSStringRef result = JSStringCreateWithUTF8CString (gResult);
    g_free(gResult);
    JSValueRef ret = JSValueMakeString (context, result);
    JSStringRelease (result);


    return ret;
}


static JSValueRef
getCustomProperty_cb (JSContextRef context,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef user_arg, prop_arg;

    if (argumentCount != 2) {
      gchar* retmessage = g_strdup_printf("Wrong argument count on call to getCustomProperty_cb. Should be 2, but was: %zu", argumentCount);
      g_message(retmessage);
      g_free(retmessage);
      return JSValueMakeNull (context);
    }



    user_arg = JSValueToStringCopy (context, arguments[0], NULL);
    prop_arg = JSValueToStringCopy (context, arguments[1], NULL);

    if (user_arg == NULL | prop_arg == NULL) {
      g_message("Error on call to getCustomProperty_cb. Arguments were NULL.");
      return JSValueMakeNull (context);
    }
    //convert strings to a *gchar so we can use it to query the prop file.
    gchar *gUsr = toGChar(user_arg);
    gchar *gProperty = toGChar(prop_arg) ;

    //After we are done using, release the strings.
    JSStringRelease (user_arg);
    JSStringRelease (prop_arg);

    gchar* userConfFilename = g_strdup_printf("%s/%s/users.conf", THEME_DIR, theme);


    JSValueRef ret = getJSValueRefFromPropFile(context, gUsr, gProperty, userConfFilename);
    g_free(userConfFilename);
    g_free(gUsr);
    g_free(gProperty);
    return ret;
}




static JSValueRef
provide_secret_cb (JSContextRef context,
                   JSObjectRef function,
                   JSObjectRef thisObject,
                   size_t argumentCount,
                   const JSValueRef arguments[],
                   JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef secret_arg;
    char secret[1024];

    // FIXME: Throw exception
    if (!(argumentCount == 1 && JSValueGetType (context, arguments[0]) == kJSTypeString))
        return JSValueMakeNull (context);

    secret_arg = JSValueToStringCopy (context, arguments[0], NULL);
    JSStringGetUTF8CString (secret_arg, secret, 1024);
    JSStringRelease (secret_arg);

    lightdm_greeter_respond (greeter, secret);

    return JSValueMakeNull (context);
}

static JSValueRef
cancel_authentication_cb (JSContextRef context,
                          JSObjectRef function,
                          JSObjectRef thisObject,
                          size_t argumentCount,
                          const JSValueRef arguments[],
                          JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);

    // FIXME: Throw exception
    if (argumentCount != 0)
        return JSValueMakeNull (context);

    lightdm_greeter_cancel_authentication (greeter);
    return JSValueMakeNull (context);
}

static JSValueRef
get_authentication_user_cb (JSContextRef context,
                            JSObjectRef thisObject,
                            JSStringRef propertyName,
                            JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    return JSValueMakeString (context, JSStringCreateWithUTF8CString (lightdm_greeter_get_authentication_user (greeter)));
}

static JSValueRef
get_is_authenticated_cb (JSContextRef context,
                         JSObjectRef thisObject,
                         JSStringRef propertyName,
                         JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    return JSValueMakeBoolean (context, lightdm_greeter_get_is_authenticated (greeter));
}

static JSValueRef
get_can_suspend_cb (JSContextRef context,
                    JSObjectRef thisObject,
                    JSStringRef propertyName,
                    JSValueRef *exception)
{
    return JSValueMakeBoolean (context, lightdm_get_can_suspend ());
}

static JSValueRef
suspend_cb (JSContextRef context,
            JSObjectRef function,
            JSObjectRef thisObject,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef *exception)
{
    // FIXME: Throw exception
    if (argumentCount != 0)
        return JSValueMakeNull (context);

    lightdm_suspend(NULL);
    return JSValueMakeNull (context);
}

static JSValueRef
get_can_hibernate_cb (JSContextRef context,
                      JSObjectRef thisObject,
                      JSStringRef propertyName,
                      JSValueRef *exception)
{
    return JSValueMakeBoolean (context, lightdm_get_can_hibernate ());
}

static JSValueRef
hibernate_cb (JSContextRef context,
              JSObjectRef function,
              JSObjectRef thisObject,
              size_t argumentCount,
              const JSValueRef arguments[],
              JSValueRef *exception)
{
    // FIXME: Throw exception
    if (argumentCount != 0)
        return JSValueMakeNull (context);

    lightdm_hibernate (NULL);
    return JSValueMakeNull (context);
}

static JSValueRef
get_can_restart_cb (JSContextRef context,
                    JSObjectRef thisObject,
                    JSStringRef propertyName,
                    JSValueRef *exception)
{
    return JSValueMakeBoolean (context, lightdm_get_can_restart ());
}

static JSValueRef
restart_cb (JSContextRef context,
            JSObjectRef function,
            JSObjectRef thisObject,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef *exception)
{
    // FIXME: Throw exception
    if (argumentCount != 0)
        return JSValueMakeNull (context);

    lightdm_restart (NULL);
    return JSValueMakeNull (context);
}

static JSValueRef
get_can_shutdown_cb (JSContextRef context,
                     JSObjectRef thisObject,
                     JSStringRef propertyName,
                     JSValueRef *exception)
{
    return JSValueMakeBoolean (context, lightdm_get_can_shutdown ());
}

static JSValueRef
shutdown_cb (JSContextRef context,
             JSObjectRef function,
             JSObjectRef thisObject,
             size_t argumentCount,
             const JSValueRef arguments[],
             JSValueRef *exception)
{
    // FIXME: Throw exception
    if (argumentCount != 0)
        return JSValueMakeNull (context);

    lightdm_shutdown (NULL);
    return JSValueMakeNull (context);
}

static JSValueRef
login_cb (JSContextRef context,
          JSObjectRef function,
          JSObjectRef thisObject,
          size_t argumentCount,
          const JSValueRef arguments[],
          JSValueRef *exception)
{
    LightDMGreeter *greeter = JSObjectGetPrivate (thisObject);
    JSStringRef arg;
    char username[1024], *session = NULL, *language = NULL;

    // FIXME: Throw exception

    arg = JSValueToStringCopy (context, arguments[0], NULL);
    JSStringGetUTF8CString (arg, username, 1024);
    JSStringRelease (arg);

    if (argumentCount > 1)
    {
        arg = JSValueToStringCopy (context, arguments[1], NULL);
        session = g_malloc (sizeof (char) * 1024);
        JSStringGetUTF8CString (arg, session, 1024);
        JSStringRelease (arg);
    }

    if (argumentCount > 2)
    {
        arg = JSValueToStringCopy (context, arguments[1], NULL);
        language = g_malloc (sizeof (char) * 1024);
        JSStringGetUTF8CString (arg, language, 1024);
        JSStringRelease (arg);
    }

    lightdm_greeter_start_session_sync (greeter, session, NULL);
    g_free (session);
    g_free (language);

    return JSValueMakeNull (context);
}

static JSValueRef
gettext_cb (JSContextRef context,
            JSObjectRef function,
            JSObjectRef thisObject,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef *exception)
{
    JSStringRef string_arg, result;
    char string[1024];

    // FIXME: Throw exception
    if (argumentCount != 1)
        return JSValueMakeNull (context);

    string_arg = JSValueToStringCopy (context, arguments[0], NULL);
    JSStringGetUTF8CString (string_arg, string, 1024);
    JSStringRelease (string_arg);

    result = JSStringCreateWithUTF8CString (gettext (string));
    return JSValueMakeString (context, result);
}

static JSValueRef
ngettext_cb (JSContextRef context,
             JSObjectRef function,
             JSObjectRef thisObject,
             size_t argumentCount,
             const JSValueRef arguments[],
             JSValueRef *exception)
{
    JSStringRef string_arg, plural_string_arg, result;
    char string[1024], plural_string[1024];
    unsigned int n;

    // FIXME: Throw exception
    if (argumentCount != 3)
        return JSValueMakeNull (context);

    string_arg = JSValueToStringCopy (context, arguments[0], NULL);
    JSStringGetUTF8CString (string_arg, string, 1024);
    JSStringRelease (string_arg);

    plural_string_arg = JSValueToStringCopy (context, arguments[1], NULL);
    JSStringGetUTF8CString (plural_string_arg, string, 1024);
    JSStringRelease (plural_string_arg);

    n = JSValueToNumber (context, arguments[2], NULL);

    result = JSStringCreateWithUTF8CString (ngettext (string, plural_string, n));
    return JSValueMakeString (context, result);
}

static const JSStaticValue lightdm_user_values[] =
{
    { "name", get_user_name_cb, NULL, kJSPropertyAttributeReadOnly },
    { "real_name", get_user_real_name_cb, NULL, kJSPropertyAttributeReadOnly },
    { "display_name", get_user_display_name_cb, NULL, kJSPropertyAttributeReadOnly },
    { "image", get_user_image_cb, NULL, kJSPropertyAttributeReadOnly },
    { "language", get_user_language_cb, NULL, kJSPropertyAttributeReadOnly },
    { "layout", get_user_layout_cb, NULL, kJSPropertyAttributeReadOnly },
    { "session", get_user_session_cb, NULL, kJSPropertyAttributeReadOnly },
    { "logged_in", get_user_logged_in_cb, NULL, kJSPropertyAttributeReadOnly },
    { NULL, NULL, NULL, 0 }
};

static const JSStaticValue lightdm_language_values[] =
{
    { "code", get_language_code_cb, NULL, kJSPropertyAttributeReadOnly },
    { "name", get_language_name_cb, NULL, kJSPropertyAttributeReadOnly },
    { "territory", get_language_territory_cb, NULL, kJSPropertyAttributeReadOnly },
    { NULL, NULL, NULL, 0 }
};

static const JSStaticValue lightdm_layout_values[] =
{
    { "name", get_layout_name_cb, NULL, kJSPropertyAttributeReadOnly },
    { "short_description", get_layout_short_description_cb, NULL, kJSPropertyAttributeReadOnly },
    { "description", get_layout_description_cb, NULL, kJSPropertyAttributeReadOnly },
    { NULL, NULL, NULL, 0 }
};

static const JSStaticValue lightdm_session_values[] =
{
    { "key", get_session_key_cb, NULL, kJSPropertyAttributeReadOnly },
    { "name", get_session_name_cb, NULL, kJSPropertyAttributeReadOnly },
    { "comment", get_session_comment_cb, NULL, kJSPropertyAttributeReadOnly },
    { NULL, NULL, NULL, 0 }
};

static const JSStaticValue lightdm_greeter_values[] =
{
    { "hostname", get_hostname_cb, NULL, kJSPropertyAttributeReadOnly },
    { "users", get_users_cb, NULL, kJSPropertyAttributeReadOnly },
    { "default_language", get_default_language_cb, NULL, kJSPropertyAttributeReadOnly },
    { "languages", get_languages_cb, NULL, kJSPropertyAttributeReadOnly },
    { "default_layout", get_default_layout_cb, NULL, kJSPropertyAttributeReadOnly },
    { "layouts", get_layouts_cb, NULL, kJSPropertyAttributeReadOnly },
    { "layout", get_layout_cb, set_layout_cb, kJSPropertyAttributeReadOnly },
    { "sessions", get_sessions_cb, NULL, kJSPropertyAttributeReadOnly },
    { "num_users", get_num_users_cb, NULL, kJSPropertyAttributeReadOnly },
    { "default_session", get_default_session_cb, NULL, kJSPropertyAttributeNone },
    { "timed_login_user", get_timed_login_user_cb, NULL, kJSPropertyAttributeReadOnly },
    { "timed_login_delay", get_timed_login_delay_cb, NULL, kJSPropertyAttributeReadOnly },
    { "authentication_user", get_authentication_user_cb, NULL, kJSPropertyAttributeReadOnly },
    { "is_authenticated", get_is_authenticated_cb, NULL, kJSPropertyAttributeReadOnly },
    { "can_suspend", get_can_suspend_cb, NULL, kJSPropertyAttributeReadOnly },
    { "can_hibernate", get_can_hibernate_cb, NULL, kJSPropertyAttributeReadOnly },
    { "can_restart", get_can_restart_cb, NULL, kJSPropertyAttributeReadOnly },
    { "can_shutdown", get_can_shutdown_cb, NULL, kJSPropertyAttributeReadOnly },
    { NULL, NULL, NULL, 0 }
};

static const JSStaticFunction lightdm_greeter_functions[] =
{
    { "cancel_timed_login", cancel_timed_login_cb, kJSPropertyAttributeReadOnly },
    { "start_authentication", start_authentication_cb, kJSPropertyAttributeReadOnly },
    { "provide_secret", provide_secret_cb, kJSPropertyAttributeReadOnly },
    { "cancel_authentication", cancel_authentication_cb, kJSPropertyAttributeReadOnly },
    { "suspend", suspend_cb, kJSPropertyAttributeReadOnly },
    { "hibernate", hibernate_cb, kJSPropertyAttributeReadOnly },
    { "restart", restart_cb, kJSPropertyAttributeReadOnly },
    { "shutdown", shutdown_cb, kJSPropertyAttributeReadOnly },
    { "login", login_cb, kJSPropertyAttributeReadOnly },
    { "getCustomProperty", getCustomProperty_cb, kJSPropertyAttributeReadOnly },
    { NULL, NULL, 0 }
};

static const JSStaticFunction gettext_functions[] =
{
    { "gettext", gettext_cb, kJSPropertyAttributeReadOnly },
    { "ngettext", ngettext_cb, kJSPropertyAttributeReadOnly },
    { NULL, NULL, 0 }
};

static const JSClassDefinition lightdm_user_definition =
{
    0,                     /* Version */
    kJSClassAttributeNone, /* Attributes */
    "LightDMUser",             /* Class name */
    NULL,                  /* Parent class */
    lightdm_user_values,       /* Static values */
};

static const JSClassDefinition lightdm_language_definition =
{
    0,                     /* Version */
    kJSClassAttributeNone, /* Attributes */
    "LightDMLanguage",         /* Class name */
    NULL,                  /* Parent class */
    lightdm_language_values,   /* Static values */
};

static const JSClassDefinition lightdm_layout_definition =
{
    0,                     /* Version */
    kJSClassAttributeNone, /* Attributes */
    "LightDMLayout",           /* Class name */
    NULL,                  /* Parent class */
    lightdm_layout_values,     /* Static values */
};

static const JSClassDefinition lightdm_session_definition =
{
    0,                     /* Version */
    kJSClassAttributeNone, /* Attributes */
    "LightDMSession",          /* Class name */
    NULL,                  /* Parent class */
    lightdm_session_values,    /* Static values */
};

static const JSClassDefinition lightdm_greeter_definition =
{
    0,                     /* Version */
    kJSClassAttributeNone, /* Attributes */
    "LightDMGreeter",          /* Class name */
    NULL,                  /* Parent class */
    lightdm_greeter_values,    /* Static values */
    lightdm_greeter_functions, /* Static functions */
};

static const JSClassDefinition gettext_definition =
{
    0,                     /* Version */
    kJSClassAttributeNone, /* Attributes */
    "GettextClass",        /* Class name */
    NULL,                  /* Parent class */
    NULL,
    gettext_functions,     /* Static functions */
};

static void
window_object_cleared_cb (WebKitWebView  *web_view,
                          WebKitWebFrame *frame,
                          JSGlobalContextRef context,
                          JSObjectRef window_object,
                          LightDMGreeter *greeter)
{
    JSObjectRef gettext_object, lightdm_greeter_object;

    gettext_class = JSClassCreate (&gettext_definition);
    lightdm_greeter_class = JSClassCreate (&lightdm_greeter_definition);
    lightdm_user_class = JSClassCreate (&lightdm_user_definition);
    lightdm_language_class = JSClassCreate (&lightdm_language_definition);
    lightdm_layout_class = JSClassCreate (&lightdm_layout_definition);
    lightdm_session_class = JSClassCreate (&lightdm_session_definition);

    gettext_object = JSObjectMake (context, gettext_class, NULL);
    JSObjectSetProperty (context,
                         JSContextGetGlobalObject (context),
                         JSStringCreateWithUTF8CString ("gettext"),
                         gettext_object, kJSPropertyAttributeNone, NULL);

    lightdm_greeter_object = JSObjectMake (context, lightdm_greeter_class, greeter);
    JSObjectSetProperty (context,
                         JSContextGetGlobalObject (context),
                         JSStringCreateWithUTF8CString ("lightdm"),
                         lightdm_greeter_object, kJSPropertyAttributeNone, NULL);
}

static void
sigterm_cb (int signum)
{
    exit (0);
}




static void logMessage(GLogLevelFlags level, const gchar* format, ... ) {
  va_list args;

  GDateTime *dateTime = g_date_time_new_now_local ();
  gchar* strDateTime = g_date_time_format(dateTime, "%F %T");
  g_date_time_unref(dateTime);

  //Alter format string, including the date/time.
  const gchar* newFormat = g_strdup_printf("%s-> %s", strDateTime, format);
  g_free(strDateTime);

  //Call g_logv with arguments sent.
  va_start (args, format);
  g_logv(G_LOG_DOMAIN, level, newFormat, args);
  va_end (args);
}


static void
resource_load_failed_cb(WebKitWebView  *web_view,
                        WebKitWebFrame *frame,
                        WebKitWebResource *web_resource,
                        GError            *err,
                        LightDMGreeter *greeter)
{
  const gchar* resourceuri = webkit_web_resource_get_uri(web_resource);
  logMessage(G_LOG_LEVEL_MESSAGE, "Error loading resource (%s): %s", resourceuri, err->message);

}

static void
load_finished_cb(WebKitWebResource *web_resource, LightDMGreeter *greeter)
{
  const gchar* resourceuri = webkit_web_resource_get_uri(web_resource);

  logMessage(G_LOG_LEVEL_MESSAGE, "I'm in load_finished_cb for: %s", resourceuri);

}

static void
load_failed_cb(WebKitWebResource *web_resource,
               GError            *err,
               LightDMGreeter *greeter)
{
  const gchar* resourceuri = webkit_web_resource_get_uri(web_resource);
  logMessage(G_LOG_LEVEL_MESSAGE, "I'm in load_failed_cb for: %s. Error: %s", resourceuri, err->message);

}

static void
response_received_cb(WebKitWebResource     *web_resource,
                     WebKitNetworkResponse *response,
                     LightDMGreeter *greeter)
{
  const gchar* resourceuri = webkit_web_resource_get_uri(web_resource);
  logMessage(G_LOG_LEVEL_MESSAGE, "I'm in response_received_cb for: %s", resourceuri);
}


static void
content_length_received_cb(WebKitWebResource *web_resource,
                           gint               length_received,
                           LightDMGreeter    *greeter)
{
  const gchar* resourceuri = webkit_web_resource_get_uri(web_resource);
  logMessage(G_LOG_LEVEL_MESSAGE, "I'm in content_length_received_cb for: %s. length: %u", resourceuri, length_received);

}

static void
header_log_cb(const char *name, const char *value, gpointer resourceuri)
{
  logMessage(G_LOG_LEVEL_MESSAGE, "I'm in header_log_cb for: %s. Header: %s", resourceuri, name);
}

//Created for debuggin. Not really using this.
static void
resource_response_received_cb(WebKitWebView         *web_view,
                                            WebKitWebFrame        *web_frame,
                                            WebKitWebResource     *web_resource,
                                            WebKitNetworkResponse *response,
                                            LightDMGreeter *greeter)
{
  const gchar* resourceuri = webkit_web_resource_get_uri(web_resource);
  logMessage(G_LOG_LEVEL_MESSAGE, "I'm in resource_response_received_cb for: %s", resourceuri);
}


//Created for debuggin. Not really using this.
static void
resource_request_starting_cb(WebKitWebView  *web_view,
                        WebKitWebFrame *frame,
                        WebKitWebResource *web_resource,
                        WebKitNetworkRequest  *request,
                        WebKitNetworkResponse *response,
                        LightDMGreeter *greeter)
{

  const gchar* resourceuri = webkit_web_resource_get_uri(web_resource);

  gboolean interessa = g_str_has_prefix(resourceuri, "http");
  if (interessa == TRUE) {
    logMessage(G_LOG_LEVEL_MESSAGE, "I'm in resource_request_starting_cb for: %s", resourceuri);

    SoupMessage *msg = webkit_network_request_get_message (request);;
    SoupMessageHeaders *headers = msg->request_headers;
    soup_message_headers_foreach (headers, header_log_cb, (gpointer) resourceuri);



    g_signal_connect (G_OBJECT (web_resource), "load-finished", G_CALLBACK (load_finished_cb), greeter);
    g_signal_connect (G_OBJECT (web_resource), "load-failed", G_CALLBACK (load_failed_cb), greeter);
    g_signal_connect (G_OBJECT (web_resource), "response-received", G_CALLBACK (response_received_cb), greeter);
    g_signal_connect (G_OBJECT (web_resource), "content-length-received", G_CALLBACK (content_length_received_cb), greeter);
  }
}

WebKitWebView*
create_web_view_cb (WebKitWebView  *web_view,
               WebKitWebFrame *frame,
               gpointer        user_data)
{
    WebKitWebSettings *settings = webkit_web_settings_new ();

    g_object_set(G_OBJECT(settings), "enable-universal-access-from-file-uris", TRUE, NULL);
    g_object_set(G_OBJECT(settings),"enable-file-access-from-file-uris", TRUE, NULL);
    g_object_set (G_OBJECT(settings), "enable-xss-auditor", FALSE, NULL);
    webkit_web_view_set_settings (WEBKIT_WEB_VIEW(web_view), settings);


    return web_view;
}

static void sethttpproxy(const gchar *httpproxy)
{
  logMessage(G_LOG_LEVEL_MESSAGE, "Setting http proxy to: %s", httpproxy);
  g_setenv("http_proxy", httpproxy, FALSE);
  g_setenv("https_proxy", httpproxy, FALSE);
}








int
main (int argc, char **argv)
{
    LightDMGreeter *greeter;
    GdkScreen *screen;
    GdkRectangle geometry;
    GKeyFile *keyfile;

    signal (SIGTERM, sigterm_cb);

    gtk_init (&argc, &argv);
    gdk_window_set_cursor (gdk_get_default_root_window (), gdk_cursor_new (GDK_LEFT_PTR));
    greeter = lightdm_greeter_new ();

    /* settings */
    GError *err = NULL;

    keyfile = g_key_file_new ();
    gboolean fileFound = g_key_file_load_from_file (keyfile, "/etc/lightdm/lightdm-tex-greeter.conf", G_KEY_FILE_NONE, &err);

    if (fileFound == FALSE) {
      theme = "angular-theme";
      logMessage(G_LOG_LEVEL_MESSAGE, "Error trying to find config for tex-greeter: %s", err->message);
      logMessage(G_LOG_LEVEL_MESSAGE, "Falling back theme to angular-theme");
      g_error_free (err);
    } else {
      //If we got the file, then try to find the theme.
      theme = g_key_file_get_string(keyfile, "greeter", "webkit-theme", &err);
      if (theme == NULL) {
        theme = "angular-theme";
        logMessage(G_LOG_LEVEL_MESSAGE, "Error attempting to get theme name with g_key_file_get_string: %s", err->message);
        logMessage(G_LOG_LEVEL_MESSAGE, "Falling to angular-theme");
        g_error_free (err);
      }

      //See if we should set the proxy.
      gchar *httpproxy = g_key_file_get_string(keyfile, "greeter", "http-proxy", &err);
      if (httpproxy != NULL) {
        sethttpproxy(httpproxy);
      } else {
        logMessage(G_LOG_LEVEL_MESSAGE, "No http-proxy in config file: %s", err->message);
        g_error_free (err);
      }
    }
    logMessage(G_LOG_LEVEL_MESSAGE, "Going with theme: %s", theme);





    g_key_file_free(keyfile);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    screen = gtk_window_get_screen (GTK_WINDOW(window));
    gdk_screen_get_monitor_geometry (screen, gdk_screen_get_primary_monitor(screen), &geometry);
    gtk_window_set_default_size (GTK_WINDOW(window), geometry.width, geometry.height);
	  gtk_window_move (GTK_WINDOW(window), geometry.x, geometry.y);

    web_view = (WebKitWebView*) webkit_web_view_new ();

    //Connect web_view signals.
    g_signal_connect (G_OBJECT (web_view), "window-object-cleared", G_CALLBACK (window_object_cleared_cb), greeter);
    g_signal_connect (G_OBJECT (web_view), "resource-load-failed", G_CALLBACK (resource_load_failed_cb), greeter);
    g_signal_connect (G_OBJECT (web_view), "create-web-view", G_CALLBACK (create_web_view_cb), greeter);

    //For debugging.
//    g_signal_connect (G_OBJECT (web_view), "resource-request-starting", G_CALLBACK (resource_request_starting_cb), greeter);
//    g_signal_connect (G_OBJECT (web_view), "resource-response-received", G_CALLBACK (resource_response_received_cb), greeter);



    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(web_view));

    //Connect greeter signals.
    g_signal_connect (G_OBJECT (greeter), "show-prompt", G_CALLBACK (show_prompt_cb), web_view);

    g_signal_connect (G_OBJECT (greeter), "show-message", G_CALLBACK (show_message_cb), web_view);

    g_signal_connect (G_OBJECT (greeter), "authentication-complete", G_CALLBACK (authentication_complete_cb), web_view);

    g_signal_connect (G_OBJECT (greeter), "autologin-timer-expired", G_CALLBACK (autologin_timeout_expired_cb), web_view);

    //Full path to index.html
    gchar* indexHtml = g_strdup_printf("file://%s/%s/index.html", THEME_DIR, theme);
    gchar* htmlFileName = g_strdup_printf("%s/%s/index.html", THEME_DIR, theme);



     logMessage(G_LOG_LEVEL_MESSAGE, "going to attempt to map file: %s", htmlFileName);

    GMappedFile * mfile = g_mapped_file_new(htmlFileName, FALSE, &err);
    if (mfile == NULL) {
      logMessage(G_LOG_LEVEL_MESSAGE, "fuck. Did not load");
    }




    WebKitWebSettings *settings = webkit_web_settings_new ();
    g_object_set(G_OBJECT(settings), "enable-universal-access-from-file-uris", TRUE, NULL);
    g_object_set(G_OBJECT(settings),"enable-file-access-from-file-uris", TRUE, NULL);

    g_object_set (G_OBJECT(settings), "enable-xss-auditor", FALSE, NULL);
    webkit_web_view_set_settings (WEBKIT_WEB_VIEW(web_view), settings);


    webkit_web_view_load_uri (WEBKIT_WEB_VIEW (web_view), indexHtml);

    gtk_widget_show_all (window);


    gboolean connect = lightdm_greeter_connect_sync (greeter, &err);
    if (err != NULL) {
      logMessage(G_LOG_LEVEL_MESSAGE, "Error on lightdm_greeter_connect_sync: %s", err->message);
      g_error_free (err);
    }

    gtk_main ();

    return 0;
}





////For testing in a gtk window
//static void destroy( GtkWidget *widget,
//                     gpointer   data )
//{
//    gtk_main_quit ();
//}
//
//int
//main (int argc, char **argv)
//{
//
//  GtkWidget *window;
//
//  gtk_init (&argc, &argv);
//
//  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
//  g_signal_connect (window, "destroy", G_CALLBACK (destroy), NULL);
//
//
//  GdkRectangle geometry;
//
//    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW(window));
//    gdk_screen_get_monitor_geometry (screen, gdk_screen_get_primary_monitor(screen), &geometry);
//    gtk_window_set_default_size (GTK_WINDOW(window), geometry.width, geometry.height);
//	  gtk_window_move (GTK_WINDOW(window), geometry.x, geometry.y);
//
//    web_view = (WebKitWebView*) webkit_web_view_new ();
//
//
//    g_signal_connect (G_OBJECT (web_view), "resource-request-starting", G_CALLBACK (resource_request_starting_cb), NULL);
//    g_signal_connect (G_OBJECT (web_view), "resource-response-received", G_CALLBACK (resource_response_received_cb), NULL);
//    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(web_view));
//
//    //Full path to index.html
//    gchar* indexHtml = g_strdup_printf("file://%s/%s/index.html", THEME_DIR, "angular-theme-test");
//    WebKitWebSettings *settings = webkit_web_settings_new ();
//
//    g_object_set(G_OBJECT(settings), "enable-universal-access-from-file-uris", TRUE, NULL);
//    g_object_set(G_OBJECT(settings),"enable-file-access-from-file-uris", TRUE, NULL);
//    g_object_set (G_OBJECT(settings), "enable-xss-auditor", FALSE, NULL);
//
//
//   logMessage(G_LOG_LEVEL_MESSAGE, "Going to try to load file: %s", indexHtml);
//    webkit_web_view_set_settings (WEBKIT_WEB_VIEW(web_view), settings);
//    webkit_web_view_load_uri (WEBKIT_WEB_VIEW (web_view), indexHtml);
//
//
//    gtk_widget_show_all (window);
//
//
//
//
//  gtk_main ();
//
//  return 0;
//
//}
