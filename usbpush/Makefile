#!/usr/bin/make
LDFLAGS=-lusb

all: usbpush

clean:
	-rm -f *.o usbpush

usbpush: usbpush.o
	$(CC) $(LDFLAGS) -o $@ $<

usbpush.o: usbpush.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONEY: all clean
