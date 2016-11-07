#include "usb_communication.h"

#include <string>
#include <vector>

namespace usb_communication {
//< magic constant determined by firmware
//< of the cypress microcontroller.
const uint32_t EP_IN = 0x86;

context init(int* error)
{
	context res;
	*error = libusb_init(&res.usb_context);
	return res;
}

void exit(context ctx)
{
	libusb_exit(ctx.usb_context);
}

void release(device device)
{
	libusb_close(device.usb_handle);
}

void claim(device device, int* error)
{
	*error = libusb_claim_interface(device.usb_handle, 0);
}

void get_devices(const context ctx, std::vector<device>& devices)
{
	const int vendor_id = 0x04b4;  //< hardcoded vendor id of cypress usb chip
	const int product_id = 0x1003; //< hardcoded product id of cypress usb chip
	libusb_device** list;
	int num = libusb_get_device_list(ctx.usb_context, &list);

	for (int i = 0; i < num; i++) {
		device d;

		if (libusb_get_device_descriptor(list[i], &d.usb_desc) < 0) {
			continue;
		}

		if (d.usb_desc.idVendor == vendor_id && d.usb_desc.idProduct == product_id) {
			d.usb_device = list[i];
			d.usb_serial_number.resize(256, 0);
			int err = libusb_open(d.usb_device, &d.usb_handle);
			if (err) {
				continue;
			}

			int len = libusb_get_string_descriptor_ascii(
			    d.usb_handle, d.usb_desc.iSerialNumber, (unsigned char*)&d.usb_serial_number[0],
			    256);

			if (len < 0) {
				continue;
			}

			d.usb_serial_number.resize(len);
			devices.push_back(d);
		}
	}

	libusb_free_device_list(list, 1);
}

int align(device device)
{
	uint8_t inbuf[512] = {};
	uint8_t outbuf[512] = {};
	libusb_device_handle* dev = device.usb_handle;
	int actual_length;
	inbuf[0] = 0xff;
	//< reset the fifos in cypress controller
	//< @todo where can one find documentation for this?
	libusb_control_transfer(dev, LIBUSB_REQUEST_TYPE_VENDOR, 0xa5, 0, 0, outbuf, 0, 10);
	libusb_bulk_transfer(dev, EP_IN, inbuf, 512, &actual_length, 10);
	libusb_bulk_transfer(dev, EP_IN, inbuf, 512, &actual_length, 10);
	libusb_bulk_transfer(dev, EP_IN, inbuf, 512, &actual_length, 10);
	libusb_bulk_transfer(dev, EP_IN, inbuf, 512, &actual_length, 10);
	libusb_reset_device(dev);
	return 0;
}
}
