mpd-notification
================

**Notify about tracks played by mpd**

This runs in background and produces notifications whenever mpd produces
an event, that is new track is played or playback is paused or stopped.
Notifications look like this:

![Notification](screenshot.png)

Requirements
------------

To compile and run `mpd-notification` you need:

* [libnotify](http://library.gnome.org/devel/notification-spec/)
* [libmpdclient](http://www.musicpd.org/libs/libmpdclient/)
* [markdown](http://daringfireball.net/projects/markdown/) (HTML documentation)
* `gnome-icon-theme` (or anything alse that include an icon named `sound`)

To use `mpd-notification` you probably want `mpd`, the
[music player daemon](http://www.musicpd.org/) itself. ;)

Some systems may require additional development packages for the libraries.
Look for `libnotify-devel`, `libmpdclient-devel` or similar.

Build and install
-----------------

Building and installing is very easy. Just run:

> make

followed by:

> make install

This will place an executable at `/usr/bin/mpd-notification`,
documentation can be found in `/usr/share/doc/mpd-notification/`.
Additionally a desktop file is installed to `/etc/xdg/autostart/`, this
automatically starts the program when logged in to a desktop environment.

Usage
-----

Just run `mpd-notification` after installation or re-login to desktop
environment for autostart.
