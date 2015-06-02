/* qt2410_boot_usb - Ram Loader for Armzone QT2410 Devel Boards
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 *
 * Modified program to be compatible with QQ/Micro/Mini2440 development boards
 * and added option to specify kernel RAM base.  Fixed minor bug where program
 * would say "Error downloading program" when in fact there was none. Code cleanup.
 * 11/10/09 by Dario Vazquez <android_04@hotmail.com>
 *
 * Modified to make it modular enough to support all boards with the same
 * protocol. Endpoint/chunksize/vendor/product variables can be passed via
 * command line. This is to support OrigenBoard.
 * 2013-07-01 by Wojciech A. Koszek <wkoszek@FreeBSD.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This program allows you to download executable code from the PC to the target
 * RAM.  After downloading it, the code can be executed.
 *
 * In order to make this work, the board must be running from the usb ram
 * loader in NOR flash rather than the regular OS in NAND.  To achieve this,
 * push move the small switch on the KERNEL board from NAND to NOR before powering
 * up the device.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>

#include <usb.h>

/*
 * Default variables (set for QT2410 to keep backward compatibility with
 * original usbpush.c
 */
#define DEFAULT_VENDOR_ID	0x5345
#define DEFAULT_PRODUCT_ID	0x1234
#define DEFAULT_OUT_EP		0x03
#define DEFAULT_IN_EP		0x81
#define KERNEL_RAM_BASE		0x30000000

static u_int16_t	g_vendor_id = DEFAULT_VENDOR_ID;
static u_int16_t	g_prod_id = DEFAULT_PRODUCT_ID;
static u_int32_t	g_out_ep = DEFAULT_OUT_EP;
static u_int32_t	g_ram_base = KERNEL_RAM_BASE;
static char		*g_fnptr = NULL;
static int		g_chunk_size = 100;

static struct usb_dev_handle *hdl;

static struct usb_device *find_device(void)
{
	struct usb_bus *bus;

	for (bus = usb_busses; bus; bus = bus->next) {
		struct usb_device *dev;
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == g_vendor_id
			    && dev->descriptor.idProduct == g_prod_id
			    && dev->descriptor.iManufacturer == 1
			    && dev->descriptor.iProduct == 2
			    && dev->descriptor.bNumConfigurations == 1
			    && dev->config->bNumInterfaces == 1
			    && dev->config->iConfiguration == 0)
				return dev;
		}
	}
	return NULL;
}


static u_int16_t dev_csum(const unsigned char *data, u_int32_t len)
{
	u_int16_t csum = 0;
	int j;

	for (j = 0; j < len; j ++) {
		csum += data[j];
	}

	return csum;
}

static int send_file(u_int32_t addr, void *data, u_int32_t len)
{
	int ret = 0;
	char *buf, *cur;
	u_int16_t csum = dev_csum(data, len);
	u_int32_t len_total = len + 10;

	printf("csum = 0x%4x\n", csum);

	/* 4 bytes address, 4 bytes length, data, 2 bytes csum */

	buf = malloc(len_total);
	if (!buf)
		return -ENOMEM;

	/* FIXME: endian safeness !!! */
	buf[0] = addr & 0xff;
	buf[1] = (addr >> 8) & 0xff;
	buf[2] = (addr >> 16) & 0xff;
	buf[3] = (addr >> 24) & 0xff;

	buf[4] = len_total & 0xff;
	buf[5] = (len_total >> 8) & 0xff;
	buf[6] = (len_total >> 16) & 0xff;
	buf[7] = (len_total >> 24) & 0xff;

	memcpy(buf+8, data, len);

	buf[len+8] = csum & 0xff;
	buf[len+9] = (csum >> 8) & 0xff;

	printf("send_file: addr = 0x%08x, len = 0x%08x\n", addr, len);

	for (cur = buf; cur < buf+len_total; cur += g_chunk_size) {
		int remain = (buf + len_total) - cur;
		if (remain > g_chunk_size)
			remain = g_chunk_size;

		ret = usb_bulk_write(hdl, g_out_ep, cur, remain, 0);
		if (ret < 0)
			break;
	}

	free(buf);

	return ret;
}

static void
usage(void)
{

	fprintf(stderr, "Usage:\n");
	fprintf(stderr,
		"\tusbpush [-a addr] [-e endpoint] [-v vendor_id] "
			"[-p product_id] -f filename\n\n"
		"Example for OrigenBoard:\n"
			"\tusbpush -e 2 -v 0x04e8 -p 0x001234 "
			"-a 0x40008000 -f <file>\n"
	);
	exit(64);
}

int
main(int argc, char **argv)
{
	struct usb_device *dev;
	char *filename, *prog;
	struct stat st;
	int fd, o;

	while ((o = getopt(argc, argv, "a:e:f:p:v:")) != -1) {
		switch (o) {
		case 'a':
			g_ram_base = strtol(optarg, NULL, 16);
			break;
		case 'e':
			g_out_ep = strtol(optarg, NULL, 16);
			break;
		case 'f':
			g_fnptr = optarg;
			break;
		case 'p':
			g_prod_id = strtol(optarg, NULL, 16);
			break;
		case 'v':
			g_vendor_id = strtol(optarg, NULL, 16);
			break;
		default:
			abort();
		}
	}
	if (g_fnptr == NULL) {
		usage();
	}
	printf("# Debug info:\n");
	printf("# filename=%s\n", g_fnptr);
	printf("# vendor_id=%#08x, product_id=%#08x, ram_base=%#08x, "
		"endpoint=%08x\n", g_vendor_id, g_prod_id, g_ram_base,
		g_out_ep);

	argc -= optind;
	argv += optind;

	/* Initialize and access USB */
	usb_init();
	if (!usb_find_busses())
		exit(1);
	if (!usb_find_devices())
		exit(1);

	dev = find_device();
	if (!dev) {
		printf("Cannot find device vendor=%08x, product=%08x in bootloader mode\n",
			g_vendor_id, g_prod_id);
		exit(1);
	}

	hdl = usb_open(dev);
	if (!hdl) {
		printf("Unable to open usb device: %s\n", usb_strerror());
		exit(1);
	}

	if (usb_claim_interface(hdl, 0) < 0) {
		printf("Unable to claim usb interface 1 of device: %s\n", usb_strerror());
		exit(1);
	}

	/* Open file */
	filename = g_fnptr;
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		printf("Unable to open file\n");
		exit(2);
	}

	if (fstat(fd, &st) < 0) {
		printf("Error to access file `%s': %s\n", filename, strerror(errno));
		exit(2);
	}


	/* mmap kernel image passed as parameter */
	prog = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!prog)
		exit(1);

	if (send_file(g_ram_base, prog, st.st_size) < 0) {
		printf("Error downloading program\n");
		exit(1);
	}
	exit(0);
}
