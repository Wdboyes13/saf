CC ?= gcc
PDOC ?= pandoc

CFLAGS  := $(shell pkg-config --cflags libzstd) \
		   -Ivendor -Ivendor/dr_libs \
		   -O2

LDFLAGS := $(shell pkg-config --libs libzstd)

to_saf: to_saf.c saf.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
saf_play: saf_play.c saf.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f saf_play to_saf

.PHONY: clean