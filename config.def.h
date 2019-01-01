/*
 * (C) 2011-2019 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* how long to display notifications */
#define NOTIFICATION_TIMEOUT	10

/* name of the icon used for notifications */
#define ICON_AUDIO_X_GENERIC	"audio-x-generic"

/* strings used to display notification messages
 * TEXT_PLAY_* need to include one string modifier '%s' each. */
#define TEXT_TOPIC		"MPD Notification"
#define TEXT_PLAY_PAUSE_STATE	"%s "
#define TEXT_PLAY_PAUSE_TITLE	"<b>%s</b>"
#define TEXT_PLAY_PAUSE_ARTIST	"by <i>%s</i>"
#define TEXT_PLAY_PAUSE_ALBUM	"from <i>%s</i>"
#define TEXT_STOP		"Stopped playback"
#define TEXT_NONE		"No action received yet."
#define TEXT_UNKNOWN		"(unknown)"

/* this is a regular expression that has to match image filename used
 * for artwork */
#define REGEX_ARTWORK	"\\(folder\\|cover\\)\\.\\(jpg\\|png\\)"

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
