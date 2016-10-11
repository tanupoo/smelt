/*
 * Copyright (C) 2000 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <time.h>

static int
settv(char *buf, char *fmt, struct timeval *tv)
{
	struct tm tm;
	char *p;

	p = strptime(buf, fmt, &tm);

	if (p == NULL)
		return -1;
	if (*p != '\0')
		return -1; //warnx("invalid format from %s with %s", buf, fmt);

	tv->tv_sec = mktime(&tm);

	return tv->tv_sec < 0 ? -1 : 0;
}

/*
 * supported tz style:
 *   +09:00 or +0900
 */
char *
strptimeval(char *str, char *fmt, struct timeval *tv)
{
	char buf[56], mbuf[10];
	char *p, *q, *m;
	long ms;
	char *bp;

	if (fmt != NULL) {
		/* non NULL fmt is not supported yet. */
		return NULL;
	}

	if (str == NULL || str[0] == '\0')
		return NULL; // warnx("string must be greater than 0.");

	tv->tv_sec = 0;
	tv->tv_usec = 0;

	p = str;
	q = buf;
	while (*p != '\0' && *p != '.')	// XXX need to check buf[] overrun.
		*q++ = *p++;

	if (*p == '\0') {
		/* no part of milliseconds */
		*q = '\0';
		return settv(buf, "%Y-%m-%dT%H:%M:%S", tv) ? NULL : p;
	}

	/* there is a milliseconds */
	m = mbuf;
	p++;
	while (*p != '\0' && *p != '+')	// XXX check need to mbuf[] overrun.
		*m++ = *p++;

	/* set milliseconds */
	*m = '\0';
	ms = strtol(mbuf, &bp, 10);
	if (*bp != '\0')
		return NULL; //warnx("invalid millseconds format %s", mbuf);
	tv->tv_usec = ms * 1000;

	/* copy the remained */
	for (; *p != '\0'; p++) {
		if (*p == ':')
			continue;	/* ignore ':' in the tz */
		*q++ = *p;
	}

	*q = '\0';
	return settv(buf, "%Y-%m-%dT%H:%M:%S%z", tv) ? NULL : p;
}
