#pragma once

#include <string>
#include <vector>

#include <libusb-1.0/libusb.h>

namespace usb_communication {
struct context
{
	libusb_context* usb_context;
};

struct device
{
	libusb_device* usb_device;
	libusb_device_descriptor usb_desc;
	std::string usb_serial_number;
	libusb_device_handle* usb_handle;
};

context init(int* error);
void get_devices(
    const usb_communication::context ctx, std::vector<usb_communication::device>& devices);
void claim(usb_communication::device device, int* error);
int align(usb_communication::device  device);
void release(usb_communication::device device);
void exit(usb_communication::context ctx);
}
