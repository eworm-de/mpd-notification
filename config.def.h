/*
 * (C) 2011-2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* name of the icon used for notifications */
#define ICON_SOUND	"sound"

/* strings used to display notification messages
 * TEXT_PLAY needs to include three '%s', in order:
 * title, artist, album */
#define TEXT_TOPIC	"MPD Notification"
#define TEXT_PLAY	"Playing <b>%s</b>\nby <i>%s</i>\nfrom <i>%s</i>"
#define TEXT_PAUSE	"Paused playback"
#define TEXT_STOP	"Stopped playback"
#define TEXT_NONE	"No action received yet."
#define TEXT_UNKNOWN	"(unknown)"

/* how to connect to mpd host ?
 * MPD_HOST is the server's host name, IP address or Unix socket path. If the
 * resolver returns more than one IP address for a host name, this functions
 * tries all of them until one accepts the connection. NULL is allowed here,
 * which will connect to the default host.
 * MPD_PORT is the TCP port to connect to, 0 for default port. If "host" is
 * a Unix socket path, this parameter is ignored.
 * MPD_TIMEOUT is the timeout in milliseconds, 0 for the default timeout. */
#define MPD_HOST	NULL
#define MPD_PORT	0
#define MPD_TIMEOUT	0

#endif /* _CONFIG_H */

// vim: set syntax=c:
