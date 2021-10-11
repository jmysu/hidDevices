/**
 * hidQtCmd
 *
 * 2021, jimmy.su / https://github.com/jmysu/hitQtCmd
 *
 *
 *
 * w/ following dependancies
 *  cxxoptions.h https://github.com/jarro2783/cxxopts
 *
 *  hidapitester.c -- Demonstrate HIDAPI via commandline
 *     2019, Tod E. Kurt / github.com/todbot
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>

#include <wchar.h>          // wprintf for HID device list
#include "HIDAPI/hidapi.h"

#include <QCoreApplication>
#include <QDebug>
#include "cxxopts.hpp"      // cmdline options

#define MAX_STR 1024        // for manufacturer, product strings
#define MAX_BUF 1024        // for buf reads & writes
uint16_t _vid = 0x2E8A;
uint16_t _pid = 0x00C0;
hid_device *dev = NULL;     // HIDAPI device we will open

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
 * list ALL HID devices connected
 */
void hidListAll()
{
	struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(0, 0);   // 0,0 = find all devices
	cur_dev = devs;
	while (cur_dev) {
		printf("%04X/%04X: %ls - %ls\n",
		       cur_dev->vendor_id, cur_dev->product_id,
		       cur_dev->manufacturer_string, cur_dev->product_string);

		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
}

/**
 * list ALL HID devices connected w/ more info.
 */
void hidListAllDetail()
{
	struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(0, 0);   // 0,0 = find all devices
	cur_dev = devs;
	while (cur_dev) {
		printf("%04X/%04X: %ls - %ls\n",
		       cur_dev->vendor_id, cur_dev->product_id,
		       cur_dev->manufacturer_string, cur_dev->product_string);

		printf("  vendorId:      0x%04hX\n", cur_dev->vendor_id);
		printf("  productId:     0x%04hX\n", cur_dev->product_id);
		printf("  usagePage:     0x%04hX\n", cur_dev->usage_page);
		printf("  usage:         0x%04hX\n", cur_dev->usage);
		printf("  serial_number: %ls \n", cur_dev->serial_number);
		printf("  interface:     %d \n", cur_dev->interface_number);
		printf("  path: %s\n", cur_dev->path);
		printf("\n");

		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
}

/**
 * open HID devices w/ VID:PID
 */
void hidOpen()
{
	//uint8_t buf[MAX_BUF];    // data buffer for send/recv
	wchar_t wstr[MAX_STR];  // string buffer for USB strings
	int res;
	if (!dev) {
		// Initialize the hidapi library
		hid_init();
		msg((char*)"Opening device, vid/pid: 0x%04X/0x%04X\n", _vid, _pid);
		dev = hid_open(_vid, _pid, NULL);
		if (dev) {
			// Read the Manufacturer String
			wstr[0] = 0x0000;
			res = hid_get_manufacturer_string(dev, wstr, MAX_STR);
			if (res < 0)
				printf("Unable to read manufacturer string\n");
			printf("Manufacturer: \"%ls\"\n", wstr);

			// Read the Product String
			wstr[0] = 0x0000;
			res = hid_get_product_string(dev, wstr, MAX_STR);
			if (res < 0)
				msg((char*)"Unable to read product string\n");
			printf("Product     : \"%ls\"\n", wstr);
		}else
			printf("[HID Open]: Device not found!\n");
	}
}

int main(int argc, char** argv)
{
	cxxopts::Options options("hidQtCmd", "A HID CLI tool!");

	options.add_options()
	   ("l,list", "List all devices", cxxopts::value<bool>()->default_value("false"))
	   ("list-detail", "List all devices w/ detail info.", cxxopts::value<bool>()->default_value("false"))
	   ("vidpid", "Change default VID:PID", cxxopts::value<std::vector<double> >())
	   ("open", "Open device w/ VID:PID", cxxopts::value<bool>()->default_value("false"))
	   ("write", "Write HID feature report [id][#1][#2][#3]...", cxxopts::value<std::vector<double> >())

	   ("q,quiet", "Print out nothing except when reading data \n", cxxopts::value<bool>()->default_value("false"))
	   ("h,help", "Print usage")
	;

	auto result = options.parse(argc, argv);
	if (result.count("help") || (argc <= 1)) {
		std::cout << options.help() << std::endl;
		exit(0);
	}

	if (result["list"].as<bool>())
		hidListAll();

	if (result["list-detail"].as<bool>())
		hidListAllDetail();

	if (result["quiet"].as<bool>())
		msg_quiet = result["quiet"].as<bool>();

	if (result.count("vidpid")) { //Change VIDPID--------------------------
		const auto values = result["vidpid"].as<std::vector<double> >();
		_vid = values[0];
		_pid = values[1];

		char buf[16];
		printf(buf, "VIDPID: %04X:%04X", _vid, _pid);
	}

	if (result["open"].as<bool>())
		hidOpen();

	//Write HID w/ Open----------------------------------------------------
	if (result.count("write")) {
		if (!dev) {
			hidOpen();
		}else {
			const auto values = result["write"].as<std::vector<double> >();
			QByteArray ba;
			for (int i = 0; i < (int)values.size(); i++) {
				uint8_t v = values[i];
				ba.append(v);
			}
			printf("HID Write:[%s]\n", ba.toHex().constData());

			msg((char*)"Writing output report of %d-bytes...", ba.size());
			int res = hid_write(dev, (const unsigned char*)ba.constData(), ba.size());
			msg((char*)"wrote %d bytes!\n", res);

			msg((char*)"Closing device...\n");
			hid_close(dev);
		}
	}

	//=====================================================================
	hid_exit(); //frees all of the static data associated with HIDAPI.

/*
   bool debug = result["debug"].as<bool>();
   qDebug() << "Debug:" << debug;

   std::string bar;
   if (result.count("bar"))
      bar = result["bar"].as<std::string>();
   qDebug() << "bar:" << bar.c_str();

   int foo = result["foo"].as<int>();
   qDebug() << "foo:" << foo;

   if (result.count("vector")) {
      std::cout << "vector = ";
      const auto values = result["vector"].as<std::vector<double> >();
      QByteArray ba;
      for (const auto& v : values) {
         std::cout << v << " ";
         ba.append(v);
      }
      std::cout << std::endl;

      //print as hex uint8 list
      std::cout << "vector = [";
      for (const uint8_t& v : values) {
         std::cout << std::hex << (0xFF & v) << " ";
      }
      std::cout << "]" << std::endl;


      qDebug() << "vector :" << ba.toHex();
   }
 */
        return 0;
}
