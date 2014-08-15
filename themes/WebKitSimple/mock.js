/*
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

// mock lightdm for testing
if (typeof lightdm == 'undefined') {
    lightdm = {};
    lightdm.hostname = "test-host";
    lightdm.languages = [
        {code: "en_US", name: "English(US)", territory: "USA"},
        {code: "en_UK", name: "English(UK)", territory: "UK"}
    ];
    lightdm.default_language = lightdm.languages[0];
    lightdm.layouts = [
        {name: "test", short_description: "test description", description: "really long epic description"}
    ];
    lightdm.default_layout = lightdm.layouts[0];
    lightdm.layout = lightdm.layouts[0];
    lightdm.sessions = [
        {key: "key1", name: "session 1", comment: "no comment"},
        {key: "key2", name: "session 2", comment: "no comment"}
    ];

    lightdm.default_session = lightdm.sessions[0];
    lightdm.authentication_user = null;
    lightdm.is_authenticated = false;
    lightdm.can_suspend = true;
    lightdm.can_hibernate = true;
    lightdm.can_restart = true;
    lightdm.can_shutdown = true;

   lightdm.getUserProperty = function(group, prop) {
      //just a mock... not actually reading from file here!
      if (group === 'raul' && prop === 'fooinfo') {
         return 'all around good guy!'
      }
      return null;

   };

    lightdm.users = [
        { name: "raul", real_name: "Raul Cesar Teixeira", display_name: "Raul Teixeira", image: "", language: "en_US", layout: null, session: null, logged_in: false},
        { name: "clarkk", real_name: "Superman", display_name: "Clark Kent", image: "", language: "en_US", layout: null, session: null, logged_in: false },
        { name: "brucew", real_name: "Batman", display_name: "Bruce Wayne", image: "batman.svg", language: "en_US", layout: null, session: null, logged_in: false},
        { name: "peterp", real_name: "Spiderman", display_name: "Peter Parker", image: "", language: "en_US", layout: null, session: null, logged_in: true}
    ];

    lightdm.sessions = [
        { name: "Gnome", key: "gnome" },
        { name: "LXQt Desktop", key: "lxqt" },
        { name: "KDE Plasma Desktop", key: "kdeplasma" }
    ];
    lightdm.default_session = lightdm.sessions[0];
    lightdm.num_users = lightdm.users.length;
    lightdm.timed_login_delay = 0; //set to a number higher than 0 for timed login simulation
    lightdm.timed_login_user = lightdm.timed_login_delay > 0 ? lightdm.users[0] : null;

    lightdm.get_string_property = function () {
    };
    lightdm.get_integer_property = function () {
    };
    lightdm.get_boolean_property = function () {
    };
    lightdm.cancel_timed_login = function () {
        _lightdm_mock_check_argument_length(arguments, 0);
        lightdm._timed_login_cancelled = true;
    };

    lightdm.provide_secret = function (secret) {
        if (typeof lightdm._username == 'undefined' || !lightdm._username) {
            throw "must call start_authentication first"
        }
        _lightdm_mock_check_argument_length(arguments, 1);
        var user = _lightdm_mock_get_user(lightdm.username);

        if (!user && secret == lightdm._username) {
            lightdm.is_authenticated = true;
            lightdm.authentication_user = user;
        } else {
            lightdm.is_authenticated = false;
            lightdm.authentication_user = null;
            lightdm._username = null;
        }
        authentication_complete();
    };

    lightdm.start_authentication = function (username) {
        _lightdm_mock_check_argument_length(arguments, 1);
        if (lightdm._username) {
            throw "Already authenticating!";
        }
        var user = _lightdm_mock_get_user(username);
        if (!user) {
            show_error(username + " is an invalid user");
        }
        show_prompt("Password: ");
        lightdm._username = username;
    };

    lightdm.cancel_authentication = function () {
        _lightdm_mock_check_argument_length(arguments, 0);
        if (!lightdm._username) {
            throw "we are not authenticating";
        }
        lightdm._username = null;
    };

    lightdm.suspend = function () {
        alert("System Suspended. Bye Bye");
        document.location.reload(true);
    };

    lightdm.hibernate = function () {
        alert("System Hibernated. Bye Bye");
        document.location.reload(true);
    };

    lightdm.restart = function () {
        alert("System restart. Bye Bye");
        document.location.reload(true);
    };

    lightdm.shutdown = function () {
        alert("System Shutdown. Bye Bye");
        document.location.reload(true);
    };

    lightdm.login = function (user, session) {
        _lightdm_mock_check_argument_length(arguments, 2);
        if (!lightdm.is_authenticated) {
            throw "The system is not authenticated";
        }
        if (user !== lightdm.authentication_user) {
            throw "this user is not authenticated";
        }
        alert("logged in successfully to session: " + session + "!!!");
        document.location.reload(true);
    };

    if (lightdm.timed_login_delay > 0) {
        setTimeout(function () {
            if (!lightdm._timed_login_cancelled()) timed_login();
        }, lightdm.timed_login_delay);
    }
}

function _lightdm_mock_check_argument_length(args, length) {
    if (args.length != length) {
        throw "incorrect number of arguments in function call";
    }
}

function _lightdm_mock_get_user(username) {
    var user = null;
    for (var i = 0; i < lightdm.users.length; ++i) {
        if (lightdm.users[i].name == username) {
            user = lightdm.users[i];
            break;
        }
    }
    return user;
}