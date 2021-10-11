/*
 #include <QCoreApplication>

   int main(int argc, char *argv[])
   {
   QCoreApplication a(argc, argv);

   return a.exec();
   }
 */

/**
 * hidapitester.c -- Demonstrate HIDAPI via commandline
 *
 * 2019, Tod E. Kurt / github.com/todbot
 *
 *
 * Add RGB LED fucntions
 *
        set <r> <g> <b>
        fade <r> <g> <b> [<speed-ms>]
        status (on|off|blink)
        blink <duty-ms> [<period-ms>]
        blink off
        off
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <wchar.h> // wprintf

#include "HIDAPI/hidapi.h"
#include <QDebug>

#define MAX_STR 1024  // for manufacturer, product strings
#define MAX_BUF 1024  // for buf reads & writes


static void print_usage(char *myname)
{
	fprintf(stderr,
		"Usage: \n"
		"  %s <cmd> [options]\n"
		"where <cmd> is one of:\n"
		"  --list                          List HID devices \n"
		"  --list-detail                   List HID devices w/ details \n"
		"  --send <datalist>               Send Ouput report to device \n"
		"  --read                          Read TCS sensor \n"
		"  --pin  <pin#> <1|0>             Set Pin# \n"
		"  --set  <r> <g> <b>              Set Neopixels color \n"
		"  --fade <r> <g> <b> [<speed-ms>] Fade Neopixels w/ <speed> \n"
		"  --base <base>                   Set decimal or hex buffer print mode\n"
		"\n"
		"", myname);
}

// local states for the "cmd" option variable
enum {
	CMD_NONE = 0,
	CMD_LIST,
	CMD_LIST_DETAIL,
	CMD_SEND_OUTPUT,
	CMD_READ_INPUT,
	CMD_CLOSE,
	CMD_PIN,
	CMD_SET_LED_COLOR,
	CMD_FADE_LED_COLOR,
};

bool msg_quiet = false;
bool msg_verbose = false;

int print_base = 16; // 16 or 10, hex or decimal
int print_width = 32; // how many characters per line
/**
 * printf that can be shut up
 */
void msg(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (!msg_quiet) {
		vprintf(fmt, args);
	}
	va_end(args);
}
/**
 * printf that is wordy
 */
void msginfo(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (msg_verbose) {
		vprintf(fmt, args);
	}
	va_end(args);
}

/**
 * print out a buffer of len bufsize in decimal or hex form
 */
void printbuf(uint8_t* buf, int bufsize, int base, int width)
{
	for (int i = 0; i < bufsize; i++) {
		if (base == 10) {
			printf(" %3d", buf[i]);
		} else if (base == 16) {
			printf(" %02X", buf[i]);
		}
		// if (i % 16 == 15 && i < bufsize-1) printf("\n");
		if (i % width == width - 1 && i < bufsize - 1) printf("\n");
	}
	printf("\n");
}

/**
 * Parse a comma-delimited 'string' containing numbers (dec,hex)
 * into a array'buffer' (of element size 'bufelem_size') and
 * of max length 'buflen', using delimiter 'delim_str'
 * Returns number of bytes written
 */
int str2buf(void* buffer, char* delim_str, char* string, int buflen, int bufelem_size)
{
	char    *s;
	int pos = 0;
	if (string == NULL) return -1;
	memset(buffer, 0, buflen); // bzero() not defined on Win32?
	while ((s = strtok(string, delim_str)) != NULL && pos < buflen) {
		string = NULL;
		switch (bufelem_size) {
		case 1:
			((uint8_t*)buffer)[pos++] = (uint8_t)strtol(s, NULL, 0); break;
		case 2:
			((int*)buffer)[pos++] = (int)strtol(s, NULL, 0); break;
		}
	}
	return pos;
}

/**
 *
 */
int main(int argc, char* argv[])
{
	uint8_t buf[MAX_BUF];    // data buffer for send/recv
	wchar_t wstr[MAX_STR];   // string buffer for USB strings
	hid_device *dev = NULL;  // HIDAPI device we will open
	int res;
	int i;
	int buflen = 64;         // length of buf in use
	int cmd = CMD_NONE;      //
	int timeout_millis = 250;

	uint16_t vid = 0x2E8A;         // productId
	uint16_t pid = 0x00C0;         // vendorId
	uint16_t usage_page = 0;  // usagePage to search for, if any
	uint16_t usage = 0;       // usage to search for, if any
	char devpath[MAX_STR];    // path to open, if filter by usage

	setbuf(stdout, NULL);   // turn off buffering of stdout

	if (argc < 2) {
		print_usage("HidTcsCmd");
		exit(1);
	}

	struct option longoptions[] =
	{
		{ "help", no_argument, 0, 'h' },
		{ "verbose", no_argument, 0, 'v' },
		{ "quiet", optional_argument, 0, 'q' },
		{ "timeout", required_argument, 0, 't' },
		{ "length", required_argument, 0, 'l' },
		{ "buflen", required_argument, 0, 'l' },
		{ "base", required_argument, 0, 'b' },
		{ "list", no_argument, &cmd, CMD_LIST },
		{ "list-detail", no_argument, &cmd, CMD_LIST_DETAIL },
		{ "read-input", optional_argument, &cmd, CMD_READ_INPUT },
		{ "send-out", required_argument, &cmd, CMD_SEND_OUTPUT },
		{ NULL, 0, 0, 0 }
	};
	const char* shortopts = "hb:";

	bool done = false;
	int option_index = 0, opt;

	while (!done) {
		memset(buf, 0, MAX_BUF);    // reset buffers
		memset(devpath, 0, MAX_STR);

		opt = getopt_long(argc, argv, shortopts, longoptions, &option_index);
		if (opt == -1) done = true; // parsed all the args
		switch (opt) {
		case 0:                       // long opts with no short opts
			if (cmd == CMD_LIST ||
			    cmd == CMD_LIST_DETAIL) {
				struct hid_device_info *devs, *cur_dev;
				//devs = hid_enumerate(vid, pid);    // 0,0 = find all devices
				devs = hid_enumerate(0, 0);    // 0,0 = find all devices
				cur_dev = devs;
				while (cur_dev) {
					if ((!usage_page || cur_dev->usage_page == usage_page) &&
					    (!usage || cur_dev->usage == usage)) {
						if (cmd == CMD_LIST) {
							printf("%04X/%04X: %ls - %ls\n",
							       cur_dev->vendor_id, cur_dev->product_id,
							       cur_dev->manufacturer_string, cur_dev->product_string);
						}else if (cmd == CMD_LIST_DETAIL) {
							printf("  vendorId:      0x%04hX\n", cur_dev->vendor_id);
							printf("  productId:     0x%04hX\n", cur_dev->product_id);
							printf("  usagePage:     0x%04hX\n", cur_dev->usage_page);
							printf("  usage:         0x%04hX\n", cur_dev->usage);
							printf("  serial_number: %ls \n", cur_dev->serial_number);
							printf("  interface:     %d \n", cur_dev->interface_number);
							printf("  path: %s\n", cur_dev->path);
							printf("\n");
						}
					}
					cur_dev = cur_dev->next;
				}
				hid_free_enumeration(devs);
			}else if (cmd == CMD_SEND_OUTPUT) {
				if (!dev) {
					// Initialize the hidapi library
					res = hid_init();
					msg("Opening device, vid/pid: 0x%04X/0x%04X\n", vid, pid);
					dev = hid_open(vid, pid, NULL);
					if (dev) {
						// Read the Manufacturer String
						wstr[0] = 0x0000;
						res = hid_get_manufacturer_string(dev, wstr, MAX_STR);
						if (res < 0)
							printf("Unable to read manufacturer string\n");
						printf("Manufacturer String: %ls\n", wstr);

						// Read the Product String
						wstr[0] = 0x0000;
						res = hid_get_product_string(dev, wstr, MAX_STR);
						if (res < 0)
							printf("Unable to read product string\n");
						printf("Product String: %ls\n", wstr);
					}
				}
				int parsedlen = str2buf(buf, ", ", optarg, sizeof(buf), 1);
				if (parsedlen < 1) {             // no bytes or error
					msg("Error: no bytes read as arg to --send...");
					break;
				}else {
					QByteArray ba = QByteArray::fromRawData((char*)buf, parsedlen);
					qDebug() << "HID Writing:" << ba.toHex();
				}

				buflen = (!buflen) ? parsedlen : buflen;

				if (!dev) {
					msg("Error on send: no device opened.\n"); break;
				}

				msg("Writing output report of %d-bytes...", parsedlen);
				res = hid_write(dev, buf, parsedlen);
				msg("wrote %d bytes:\n", res);
			}else if (cmd == CMD_READ_INPUT) {
				if (!dev) {
					msg("Opening device, vid/pid: 0x%04X/0x%04X\n", vid, pid);
					dev = hid_open(vid, pid, NULL);
				}
				if (!dev) {
					msg("Error on read: no device opened.\n"); break;
				}
				if (!buflen) {
					msg("Error on read: buffer length is 0. Use --len to specify.\n"); break;
				}
				uint8_t report_id = (optarg) ? strtol(optarg, NULL, 10) : 0;
				{
					msg("Reading %d-byte input report %d, %d msec timeout...",
					    buflen, report_id, timeout_millis);
					res = hid_read_timeout(dev, buf, buflen, timeout_millis);
					msg("read %d bytes:\n", res);
					if (res > 0) {
						printbuf(buf, buflen, print_base, print_width);
						memset(buf, 0, buflen);      // clear it out
					}else if (res == -1) {       // removed device
						break;
					}
				}
			}
			break;    // case 0 (longopts without shortops)
		case 'h':
			print_usage("hidapiTCS");
			break;
		case 'b':
			print_base = strtol(optarg, NULL, 10);
			msginfo("Set print_base to %d\n", print_base);
			break;
		}   // switch(opt)
	}  // while(!done)

	if (dev) {
		msg("Closing device\n");
		hid_close(dev);
	}
	res = hid_exit();
} // main
