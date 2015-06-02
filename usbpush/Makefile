#!/usr/bin/make
LDFLAGS=-lusb

all: usbpush

clean:
	-rm -f *.o usbpush

usbpush: usbpush.o
	$(CC) -o $@ $< $(LDFLAGS) 

usbpush.o: usbpush.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONEY: all clean
