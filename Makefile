#
# you should define TARGETS in your Makefile so that useful target
# in this file works properly.
#
#     TARGETS
#     TEST_TARGETS
#     DIRS
#     DEFS
#         this environment variable will be passed to the Makefile in DIRS.
#     OBJS
#     RMFILES
#
# you must include this Makefile.common immediately after definition
# of above variables.
#
# Your Makefile shoud have the following line if you want
# to define the OS specific parameters.
#
#     ifeq ($(OS),Linux)
#     LDLIBS+= -lpthread
#     endif
#
# CPPFLAGS
#   CFLAGS
#  LDFLAGS
#   LDLIBS
#

SMELT_LIB = libsmelt.a
TV_EXE = test_tv
TS_EXE = test_ts
TV_OBJS = test_tv.o $(SMELT_LIB)
TS_OBJS = test_ts.o $(SMELT_LIB)

TARGETS = $(SMELT_LIB) $(TV_EXE) $(TS_EXE)

.PHONY: clean _dirs_

ifndef OS
OS = $(shell uname -s)
endif
ifndef ARCH
ARCH = $(shell arch)
endif
 
CPPFLAGS += -I.
CPPFLAGS += -I/usr/local/include

# libuv
CPPFLAGS += -I/Users/sakane/work/03Cisco/02Product/EAF/sdk/sdk-dslink-c/deps/libuv/include

CFLAGS += -Werror -Wall
CFLAGS += -g
#CFLAGS += -O2

LDFLAGS	+= -L/usr/local/lib
LDFLAGS	+= -L.

# libuv
LDFLAGS += -L/Users/sakane/work/03Cisco/02Product/EAF/sdk/sdk-dslink-c/build

LDLIBS += -lsmelt
LDLIBS += -llibuv

all: _dirs_ $(TARGETS)

_dirs_:
ifdef	DIRS
	for d in $(DIRS); do if test -d $$d ; then (cd $$d ; $(DEFS) make); fi ; done
endif

$(TS_EXE): $(TS_OBJS)

$(TV_EXE): $(TV_OBJS)

$(SMELT_LIB): smelt.o strptimeval.o strftimeval.o histgram.o
	ar rv $@ $^

#
#.SUFFIXES: .o .c
#
#.c.o:
#	$(CC) $(CFLAGS) -c $<

clean:
	-rm -rf a.out *.dSYM *.o
ifneq ($(RMFILES),)
	-rm -rf $(RMFILES)
endif
ifdef	DIRS
	for d in $(DIRS) ; do (cd $$d ; make clean); done
endif
ifneq ($(TARGETS),)
	-rm -f $(TARGETS)
endif
ifneq ($(TEST_TARGETS),)
	-rm -f $(TEST_TARGETS)
endif

distclean: clean
	-rm -f config.cache config.status config.log .depend
	if test -f Makefile.ini ; then rm -f Makefile ; fi

show-options:
	@echo CPPFLAGS =$(CPPFLAGS)
	@echo   CFLAGS =$(CFLAGS)
	@echo  LDFLAGS =$(LDFLAGS)
	@echo   LDLIBS =$(LDLIBS)

