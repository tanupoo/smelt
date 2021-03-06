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

static int f_debug = 0;

int
get_min_max(double *data, size_t n_data, double *min, double *max)
{
	int i;

	if (!n_data)
		return -1;

	*min = data[0];
	*max = data[0];
	for (i = 1; i < n_data; i++) {
		*min = *min > data[i] ? data[i] : *min;
		*max = *max < data[i] ? data[i] : *max;
	}

	return 0;
}

int
get_max(double *data, size_t n_data, double *max)
{
	int i;

	if (!n_data)
		return -1;

	*max = data[0];
	for (i = 1; i < n_data; i++)
		*max = *max < data[i] ? data[i] : *max;

	return 0;
}

int
get_min(double *data, size_t n_data, double *min)
{
	int i;

	if (!n_data)
		return -1;

	*min = data[0];
	for (i = 1; i < n_data; i++)
		*min = *min > data[i] ? data[i] : *min;

	return 0;
}

#define MAX_HIST_SIZE	60

void
print_histgram(int *hist, size_t n_hist, double min, double tick)
{
	int i, j;
	double t;
	int max_hist, max_res, max_val;
	char buf[24];
	char fmt[24];

	if (!n_hist) {
		warn("ERROR: no histgram was found.");
		return;
	}

	max_hist = hist[0];
	max_val = snprintf(buf, sizeof(buf), "%d", hist[0]);
	t = min;
	for (i = 1; i < n_hist; i++) {
		max_hist = max_hist > hist[i] ? max_hist : hist[i];
		j = snprintf(buf, sizeof(buf), "%d", hist[i]);
		max_val = max_val > j ? max_val : j;
		t += tick;
	}
	max_res = snprintf(buf, sizeof(buf), "%.2f", t);
	snprintf(fmt, sizeof(fmt), "%%%d.2f: %%%dd: ", max_res, max_val);

	t = min;
	for (i = 0; i < n_hist; i++) {
		printf(fmt, t, hist[i]);
		for (j = 0; j < (hist[i] * MAX_HIST_SIZE) / max_hist; j++)
			printf("*");
		printf("\n");
		t += tick;
	}
}

int
get_histgram(double *data, size_t n_data, size_t scale,
		int **hist, double *min, double *tick)
{
	double max;
	int n, i;

	if (!n_data || !scale)
		return -1;

	if ((*hist = calloc(scale, sizeof(int))) == NULL)
		return -1;
	for (n = 0; n < scale; n++)
		(*hist)[n] = 0;

	get_min_max(data, n_data, min, &max);
	*tick = (max - *min) / scale;
	*tick = *tick ? *tick : 1;
	for (i = 0; i < n_data; i++) {
		n = (int)((data[i] - *min) / *tick);
		if (f_debug)
			printf("data[%d] = %f n=%d\n", i, data[i], n);
		if (n >= scale) {
			if (f_debug)
				printf("** added to the end of the hist **\n");
			(*hist)[n - 1]++;
			continue;
		}
		(*hist)[n]++;
	}

	return 0;
}

