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

#include <time.h>	/* for not cauing sngle process */
#include <sys/time.h>	/* for gettimeofday() */

#if 0
size_t
strftimeval(char *buf, size_t bufsize, const char *format,
		const struct tm *tm)
{
	char *bp, *mp;
	int rest_len;
	int len;
	struct timeval tv;
	char s_msec[5];

	if (format == NULL)
		format = "%Y-%m-%dT%H:%M:%S%z";

	bp = buf;
	rest_len = bufsize;
	len = strftime(bp, rest_len, format, tm);
	if (len == 0)
		return 0;

	bp += len;
	rest_len -= len;

	gettimeofday(&tv, NULL);
	len = snprintf(s_msec, sizeof(s_msec), ".%03d", tv.tv_usec / 1000);
	if (len == 0)
		return 0;

	rest_len -= len;
	if (rest_len <= 0)
		return 0;

	// copy the part from '+' to '\0' into the tail.
	do {
		*(bp + 4) = *bp;
	} while (bp != buf && *bp-- != '+');
	if (bp == buf)
		return 0;
	for (mp = s_msec; *mp != '\0'; mp++)
		*++bp = *mp;

	return bufsize - rest_len;
}
#else
size_t
strftimeval(char *buf, size_t bufsize, const char *format,
		const struct tm *tm)
{
	char *bp;
	int rest_len;
	int len;
	struct timeval tv;

	if (format == NULL)
		format = "%Y-%m-%dT%H:%M:%S";

	bp = buf;
	rest_len = bufsize;
	len = strftime(bp, rest_len, format, tm);
	if (len == 0)
		return 0;

	bp += len;
	rest_len -= len;
	gettimeofday(&tv, NULL);
	len = snprintf(bp, rest_len, ".%03d", tv.tv_usec / 1000);
	if (len == 0)
		return 0;

	bp += len;
	rest_len -= len;
	len = strftime(bp, rest_len, "%z", tm);
	if (len == 0)
		return 0;

	return bufsize - rest_len + len;
}
#endif

size_t
strftimeval_current(char *buf, size_t bufsize, const char *format)
{
	time_t t = time(NULL);
	struct tm tm;

	return strftimeval(buf, bufsize, format, localtime_r(&t, &tm));
}
