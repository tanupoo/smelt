#include "strftimeval.h"

#define GW_INTERVAL_DEFAULT	10000	/* 10 (ms) */
#define GW_COUNT_MAX_DEFAULT	1000	/* 1000 (times) */

struct smelt_info {
	uint32_t mid;        /* message identifier */
	uint32_t len_hdr;    /* data length included in the header. */
	uint32_t len_sys;     /* data length returned by a system call. */
	struct timeval tv;
	char *ts;
};

struct smelt {
	/* tx */
	struct smelt_info *tx_stat;
	uint32_t tx_index;    /* current index of tx_stat */
	uint32_t tx_count;    /* the number of messages has been transmitted. */
	uint32_t tx_last_mid; /* identifier the last transmitted. */
	/* rx */
	struct smelt_info *rx_stat;
	uint32_t rx_index;    /* current index of rx_stat */
	uint32_t rx_count;    /* the number of messages has been received. */
	/* flags */
	int finished;	      /* flag if the session has been done. */
	/*
	 *       start   end
	 * 0x00: tv      tv
	 * 0x01: ts      tv
	 * 0x02: tv      ts
	 * 0x03: ts      ts
	 */
#define SMELT_MODE_S_TS  0x01
#define SMELT_MODE_E_TS  0x02
	int f_mode;
	uint32_t max_count;
	int f_csv;
	int debug_level;
};

void smelt_print_result(struct smelt *);
void smelt_start_tv(struct smelt *, struct smelt_info *);
void smelt_end_tv(struct smelt *, struct smelt_info *);
void smelt_start_ts(struct smelt *, struct smelt_info *, char *);
void smelt_end_ts(struct smelt *, struct smelt_info *, char *);
struct smelt *smelt_init(int, uint32_t, int, int, int);
