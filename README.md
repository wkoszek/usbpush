usbpush
=======

USBpush program for "DNW" U-boot protocol support in
FriendlyARM/QT2410/OrigenBoard boards. It will let you upload data to the
CPU's target memory via USB programmer embedded into the board.

Supported boards:
- FriendlyARM modules
- OrigenBoard

To use it, you must have U-boot loader with `dnw` command. For OrigenBoard,
I used following release from InSignal's forum:

	141037562 Jun 28 19:08 origen_quad-jb_mr1.1-20130625-es2.tar.gz

`usbpush/` directory contains `usbpush` program. The other directories
aren't supported and are here just for backward compatibility and to be able
to compare differences from the original release.

Usage
=====

Following is the usage of `usbpush`:

	usbpush [-a addr] [-e endpoint] [-v vendor_id] [-p product_id] -f filename

`addr` is the destination address in the target CPU's memory

`endpoint` is the endpoint number which board will listen on upon 'dnw'
command is started in U-boot.

`vendor_id` is the vendor ID read from `lsusb` upon 'dnw' command is run in
U-boot

`product_id` is similar to vendor ID.

`filename` if the file which you want to upload to the memory of the CPU.

Example for OrigenBoard:

	usbpush -e 2 -v 0x04e8 -p 0x001234 -a 0x40008000 -f <file>
