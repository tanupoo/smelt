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
#include <uv.h>
#include <arpa/inet.h>

#ifdef __linux__
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "depends.h"
#include "smelt.h"

#define DEFAULT_SERVER_ADDRESS "127.0.0.1"
#define DEFAULT_SERVER_PORT "9999"

int interval = 1;
uint32_t max_count = 1000;
int f_debug = 0;

char *prog_name = NULL;

void
usage()
{
	printf(
"Usage: %s [-i interval] [-m max_count] [-dh]\n"
	, prog_name);

	exit(0);
}

void
on_timer_close(uv_handle_t *handle)
{
	printf("%s\n", __FUNCTION__);
}

void
on_timer(uv_timer_t *handle)
{
	struct smelt *mt = (struct smelt *)handle->data;
	struct smelt_info mi;
	static int mid = 1;

	if (mt->tx_count >= max_count) {
		printf("%s: reached to max count.\n", __FUNCTION__);
		uv_timer_stop(handle);
		return;
	}

	mi.mid = mid;
	mi.len_hdr = 1;
	mi.len_sys = 1;

	smelt_start_tv(mt, &mi);
	usleep(1000);
	smelt_end_tv(mt, &mi);

	mid++;
}

uv_loop_t main_loop;

int
run()
{
	uv_timer_t timer;
	struct smelt *mt;

	printf("interval=%d max_count=%d\n", interval, max_count);

	/* initialize */
	mt = smelt_init(0, max_count, 1, 0, 1);

	uv_loop_init(&main_loop);

	uv_timer_init(&main_loop, &timer);
	timer.data = (void *)mt;
	timer.close_cb = on_timer_close;
	uv_timer_start(&timer, on_timer, 0, interval);
	uv_run(&main_loop, UV_RUN_DEFAULT);

	uv_timer_stop(&timer);
	uv_close((uv_handle_t *)&timer, on_timer_close);
	uv_loop_close(&main_loop);

	/* finalize */
	smelt_print_result(mt);

	return 0;
}

int
main(int argc, char *argv[])
{
	int ch;

	prog_name = 1 + rindex(argv[0], '/');

	while ((ch = getopt(argc, argv, "i:m:dh")) != -1) {
		char *bp;
		switch (ch) {
		case 'i':
			interval = strtol(optarg, &bp, 10);
			break;
		case 'm':
			max_count = strtol(optarg, &bp, 10);
			break;
		case 'd':
			f_debug++;
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	run();

	return 0;
}

