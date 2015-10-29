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

This directory
==============

`usbpush/` directory contains `usbpush` program. The other directories
aren't supported and are here just for backward compatibility and to be able
to compare differences from the original release.

Use case
========

In U-boot:

	U-Boot 2010.12 (Jan 10 2013 - 15:10:44) for Insignal ORIGEN


	CPU: S5PC220 [Samsung SOC on SMP Platform Base on ARM CortexA9]
	APLL = 1000MHz, MPLL = 800MHz
	DRAM:  1023 MiB
	TrustZone Enabled BSP
	BL1 version: 20121128


	Checking Boot Mode ... EMMC4.41
	REVISION: 1.1
	MMC Device 0: 3816 MB
	MMC Device 1: 7580 MB
	MMC Device 2 not found
	Hit any key to stop autoboot:  0 

Run `dnw` command:

	ORIGEN # dnw
	OTG cable Connected!
	Now, Waiting for DNW to transmit data

On the PC with GNU/Linux to which the board is plugged, type (for
OrigenBoard):

	usbpush -e 2 -v 0x04e8 -p 0x001234 -a 0x40008000 -f <file>

Example output on the PC:

	# Debug info:
	# filename=usbpush
	# vendor_id=0x0004e8, product_id=0x001234, ram_base=0x40008000,
	endpoint=00000002
	csum = 0x66f6
	send_file: addr = 0x40008000, len = 0x00002664

Output on OrigenBoard:

	Download Done!! Download Address: 0xc0000000, Download Filesize:0x2664
	Checksum is being calculated.
	Checksum O.K.
	ORIGEN # 

# Author

- Wojciech Adam Koszek, [wojciech@koszek.com](mailto:wojciech@koszek.com)
- [http://www.koszek.com](http://www.koszek.com)
