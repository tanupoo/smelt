#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <math.h>

#include "smelt.h"
#include "strptimeval.h"
#include "histgram.h"
#include "depends.h"

#define AP_M 1000000

/* XXX thread unsafe */
/* holder for the signal caught */
static struct smelt *saved_smelt = NULL;

/*
 * @brief calculate the diff := tv_end - tv_start
 * @return the diff in microseconds.
 */
static long
smelt_timeval_diff(struct timeval *tv_start, struct timeval *tv_end)
{
	if (tv_end->tv_sec == tv_start->tv_sec) {
		if (tv_end->tv_usec < tv_start->tv_usec)
			return -1;
		return tv_end->tv_usec - tv_start->tv_usec;
	}
	if (tv_end->tv_sec < tv_start->tv_sec)
		return -1;
	if (tv_end->tv_usec < tv_start->tv_usec) {
		return (tv_end->tv_sec - 1 - tv_start->tv_sec) * AP_M +
			tv_end->tv_usec + AP_M - tv_start->tv_usec;
	}
	return (tv_end->tv_sec - tv_start->tv_sec) * AP_M +
		tv_end->tv_usec - tv_start->tv_usec;
}

static void
smelt_canon(struct smelt *mt)
{
	int i;

	if (mt->f_mode & SMELT_MODE_S_TS) {
		for (i = 0; i < mt->tx_index; i++) {
			strptimeval(mt->tx_stat[i].ts, NULL, &(mt->tx_stat[i].tv));

		}
	}

	if (mt->f_mode & SMELT_MODE_E_TS) {
		for (i = 0; i < mt->rx_index; i++) {
			strptimeval(mt->rx_stat[i].ts, NULL, &(mt->rx_stat[i].tv));
		}
	}
}

/**
 * @brief print the measurement result.
 */
void
smelt_print_result(struct smelt *mt)
{
	int found;
	double *diff;
	long sum, max, min;	/* rtt */
	long prev_tv, si_sum;	/* sampling interval */
	long sum_len;	/* pkt len */
	int loss;
	int i, j;
	char fmt_header[128];

	/* allocate the maximum number of diffs */
	if ((diff = calloc(mt->tx_index, sizeof(double))) == NULL)
		err(1, "ERROR: calloc(diff)");

	/*
	 * canonicalize: set ts into tv.
	 */
	smelt_canon(mt);

	/*
	 *  stat for 127.0.0.1/9999:
	 *    # of send     : 1
	 *    # of recv     : 1
	 *    loss packets  : 0 (0.0 %)
	 *    samp int. (us): 0.0
	 *    rtt min (us)  : 84
	 *    rtt max (us)  : 84
	 *    rtt mean (us)  : 84
	 *    mean pktlen (B): 1280.0
	 */
	sum = max = min = sum_len = 0;
	prev_tv = si_sum = 0;
	loss = 0;
	for (i = 0; i < mt->tx_index; i++) {
		found = 0;
		for (j = 0; j < mt->rx_index; j++) {
			if (mt->tx_stat[i].mid != mt->rx_stat[j].mid)
				continue;
			/* match */
			found++;
			diff[i] = smelt_timeval_diff(
				&mt->tx_stat[i].tv, &mt->rx_stat[j].tv);
			if (diff[i] < 0) {
				printf("ERROR: diff is negative.\n");
				printf("id=%d s=" FMT_TV "\n",
					mt->tx_stat[i].mid,
					mt->tx_stat[i].tv.tv_sec,
					mt->tx_stat[i].tv.tv_usec);
				printf("id=%d r=" FMT_TV "\n",
					mt->rx_stat[j].mid,
					mt->rx_stat[j].tv.tv_sec,
					mt->rx_stat[j].tv.tv_usec);
			}
			if (sum == 0) {
				max = diff[i];
				min = diff[i];
				sum = diff[i];
				si_sum = 0;
				prev_tv = mt->rx_stat[j].tv.tv_sec * AP_M +
					mt->rx_stat[j].tv.tv_usec;
				sum_len = mt->rx_stat[j].len_hdr;
			} else {
				sum += diff[i];
				max = diff[i] > max ? diff[i] : max;
				min = diff[i] < min ? diff[i] : min;
				si_sum += mt->rx_stat[j].tv.tv_sec * AP_M +
					mt->rx_stat[j].tv.tv_usec - prev_tv;
				prev_tv = mt->rx_stat[j].tv.tv_sec * AP_M +
					mt->rx_stat[j].tv.tv_usec;
				sum_len += mt->rx_stat[j].len_hdr;
			}
			break;
		}
		if (!found)
			loss++;
	}

	if (mt->tx_index - loss != mt->rx_index)
		printf("WARN: #of TX + loss != #of RX\n");

	/* output */
	if (mt->f_csv) {
		printf(fmt_header, "target");
		printf(" %-5d %-5d %-5d %.1f %.1f %ld %ld %ld %.1f\n",
			mt->tx_index, mt->rx_index,
			loss, 100*loss/(float)mt->tx_index,
			mt->rx_index == 1 ? 0 : si_sum/(float)(mt->rx_index-1),
			min, max, mt->rx_index == 0 ? -1 : sum/(mt->rx_index),
			mt->rx_index == 0 ? -1 : sum_len/(float)mt->rx_index);
	} else {
		printf("  # of send     : %d\n", mt->tx_index);
		printf("  # of recv     : %d\n", mt->rx_index);
		printf("  loss packets  : %d (%.1f %%)\n",
			loss, 100*loss/(float)mt->tx_index);
		/* sampling interval average */
		printf("  samp int. (us): %.1f\n",
			mt->rx_index == 1 ?
			0 : si_sum/(float)(mt->rx_index-1));
		printf("  rtt min (us)  : %ld\n", min);
		printf("  rtt max (us)  : %ld\n", max);
		printf("  rtt mean (us) : %.3f\n",
			mt->rx_index == 0 ? -1 : sum/(float)mt->rx_index);
		printf("  mean pktlen (B): %.1f\n",
			mt->rx_index == 0 ? -1 : sum_len/(float)mt->rx_index);
	}

	/* histgram */
	if (mt->rx_index) {
		int *hist;
		double tick;

		get_histgram(diff, mt->tx_index - loss, 10, &hist, &tick);
		print_histgram(hist, 10, tick);
	}
}

/*
 * record the tick of the start.
 */
void
smelt_start_tv(struct smelt *mt, struct smelt_info *mi)
{
	mt->tx_stat[mt->tx_index].mid = mi->mid;
	mt->tx_stat[mt->tx_index].len_hdr = mi->len_hdr;
	mt->tx_stat[mt->tx_index].len_sys = mi->len_sys;
	gettimeofday(&mt->tx_stat[mt->tx_index].tv, NULL);
	mt->tx_last_mid = mi->mid;
	mt->tx_count++;
	mt->tx_index++;
}

/*
 * record the tick of the end.
 */
void
smelt_end_tv(struct smelt *mt, struct smelt_info *mi)
{
	mt->rx_stat[mt->rx_index].mid = mi->mid;
	mt->rx_stat[mt->rx_index].len_hdr = mi->len_hdr;
	mt->rx_stat[mt->rx_index].len_sys = mi->len_sys;
	gettimeofday(&mt->rx_stat[mt->rx_index].tv, NULL);
	mt->rx_count++;
	mt->rx_index++;
}

/*
 * 2016-10-03T20:52:33.601+09:00
 */
void
smelt_start_ts(struct smelt *mt, struct smelt_info *mi, char *ts)
{
	mt->tx_stat[mt->tx_index].mid = mi->mid;
	mt->tx_stat[mt->tx_index].len_hdr = mi->len_hdr;
	mt->tx_stat[mt->tx_index].len_sys = mi->len_sys;
	strcpy(mt->tx_stat[mt->tx_index].ts, ts);
	mt->tx_last_mid = mi->mid;
	mt->tx_count++;
	mt->tx_index++;
}

void
smelt_end_ts(struct smelt *mt, struct smelt_info *mi, char *ts)
{
	mt->rx_stat[mt->rx_index].mid = mi->mid;
	mt->rx_stat[mt->rx_index].len_hdr = mi->len_hdr;
	mt->rx_stat[mt->rx_index].len_sys = mi->len_sys;
	strcpy(mt->rx_stat[mt->rx_index].ts, ts);
	mt->rx_count++;
	mt->rx_index++;
}

static void
smelt_signal_int(int sig)
{
	printf(" ...\n");
	smelt_print_result(saved_smelt);
	exit(0);
}

/*
 * max_count: how many times to test.
 * f_csv: if 1, the result will be printed in csv format.
 * debug: debug level.
 */
struct smelt *
smelt_init(int f_mode, uint32_t max_count, int use_signal,
		int f_csv, int debug_level)
{
	struct smelt *new;

	if (max_count == 0)
		errx(1, "ERROR: count must be more than 1.");

	if ((new = calloc(1, sizeof(struct smelt))) == NULL)
		err(1, "ERROR: %s: calloc(smelt)", __FUNCTION__);
	new->finished = 0;
	new->max_count = max_count;
	new->f_csv = f_csv;
	new->debug_level = debug_level;
	new->f_mode = f_mode;

	new->tx_stat = calloc(max_count, sizeof(struct smelt_info));
	if (new->tx_stat == NULL)
		err(1, "ERROR: %s: calloc(smelt_info)", __FUNCTION__);

	if (f_mode & SMELT_MODE_S_TS) {
		int i;
		/* 1 + strlen("2016-10-03T20:52:33.601+09:00"); */
		int buflen = 48;
		for (i = 0; i < max_count; i++) {
			if ((new->tx_stat[i].ts = malloc(buflen)) == NULL)
				err(1, "malloc(smelt_init)");
		}
	}

	new->rx_stat = calloc(max_count, sizeof(struct smelt_info));
	if (new->rx_stat == NULL)
		err(1, "ERROR: %s: calloc(smelt_info)", __FUNCTION__);

	if (f_mode & SMELT_MODE_E_TS) {
		int i;
		/* 1 + strlen("2016-10-03T20:52:33.601+09:00"); */
		int buflen = 48;
		for (i = 0; i < max_count; i++) {
			if ((new->rx_stat[i].ts = malloc(buflen)) == NULL)
				err(1, "malloc(smelt_init)");
		}
	}

	if (use_signal && signal(SIGINT, smelt_signal_int) == SIG_ERR)
		err(1, "ERROR: %s: signal(SIGINT)", __FUNCTION__);

	return new;
}
