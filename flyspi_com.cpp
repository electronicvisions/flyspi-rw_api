#include <rw_api/flyspi_com.h>

namespace rw_api {

	void usb_transfer_callback(libusb_transfer* trans);
	void usb_resp_transfer_callback(libusb_transfer* trans);

	//---------------------------------------------------------------------------
	FlyspiCom::FlyspiCom(std::string serial) : usb(usbcomm::warning)
	{
		if( usb.status != 0 ) {
			std::stringstream strm;
			strm << "Initialising libusb failed with error code "
				<< usb.status;
			throw DeviceError(__PRETTY_FUNCTION__, strm.str());
		}

		if( usb.open(usb_vendor_id, usb_product_id, serial) != 0 ) {
			std::stringstream strm;
			strm << "Failed to open USB device with vendor ID: 0x"
				<< std::hex << usb_product_id
				<< " and product ID: 0x"
				<< std::hex << usb_vendor_id;
			throw DeviceError(__PRETTY_FUNCTION__, strm.str());
		}

		usb.align();
		usb_serial = serial;
	}
	FlyspiCom::FlyspiCom()
		:	usb(usbcomm::warning)
	{
		if( usb.status != 0 ) {
			std::stringstream strm;
			strm << "Initialising libusb failed with error code "
				<< usb.status;
			throw DeviceError(__PRETTY_FUNCTION__, strm.str());
		}

		if( usb.open_first(usb_vendor_id, usb_product_id) != 0 ) {
			std::stringstream strm;
			strm << "Failed to open USB device with vendor ID: 0x"
				<< std::hex << usb_product_id
				<< " and product ID: 0x"
				<< std::hex << usb_vendor_id;
			throw DeviceError(__PRETTY_FUNCTION__, strm.str());
		}

		usb.align();
		usb_serial = usb.getSerial();
	}
	//---------------------------------------------------------------------------
	FlyspiCom::RequestHandle	
	FlyspiCom::commit(Locator const& loc,
				BufferPtr const& queryBuffer,
				unsigned int const queryBufferSize,
				BufferPtr const& respBuffer,
				unsigned int const respBufferSize) 
	{
		//std::cout << (boost::format("Submitting transfers with query size: %d, resp size: %d\n")
				//% queryBufferSize % respBufferSize).str();
		//std::cout << (boost::format("[0x%08x 0x%08x 0x%08x 0x%08x 0x%08x...]")
				//% queryBuffer[0] % queryBuffer[1] % queryBuffer[2] % queryBuffer[3] % queryBuffer[4]).str() << std::endl;

		//auto success = usb.bulktrans((unsigned char*)queryBuffer, 
				//(unsigned char*)respBuffer,
				//queryBufferSize * sizeof(BufferType),
				//respBufferSize * sizeof(BufferType));

		//if( success != 0 )
			//throw DeviceError(__PRETTY_FUNCTION__,
					//(boost::format("failed to commit query (status = %d)")
					 //% success).str());

		static unsigned int const chunk_size = 512;

		RequestHandle rv(new RequestHandleInfo);
		int success;

		if( ((queryBufferSize * sizeof(BufferType)) % chunk_size != 0)
				|| ((respBufferSize * sizeof(BufferType)) % chunk_size != 0) )
			throw ImplementationError(__PRETTY_FUNCTION__,
					(boost::format("buffers must be a multiple of %d bytes large")
					 % chunk_size).str());

		auto ptr = reinterpret_cast<unsigned char*>(queryBuffer);
		auto ptr_size = queryBufferSize * sizeof(BufferType);

		rv->transfer = libusb_alloc_transfer(0);
		if( rv->transfer == nullptr )
			throw DeviceError(__PRETTY_FUNCTION__, "failed to allocate transfer object");

		libusb_fill_bulk_transfer(rv->transfer,
				usb.mydev,
				ep_out,
				ptr,
				ptr_size,
				usb_transfer_callback,
				rv.get(),
				usb_timeout_ms);

		success = libusb_submit_transfer(rv->transfer);
		if( success != 0 ) {
			throw DeviceError(__PRETTY_FUNCTION__,
					(boost::format("submitting USB transfer failed with error code %d")
					 % success).str());
		}

		ptr = reinterpret_cast<unsigned char*>(respBuffer);
		ptr_size = respBufferSize * sizeof(BufferType);

		rv->resp_transfer = libusb_alloc_transfer(0);
		if( rv->resp_transfer == nullptr )
			throw DeviceError(__PRETTY_FUNCTION__, "failed to allocate response transfer object");

		libusb_fill_bulk_transfer(rv->resp_transfer,
				usb.mydev,
				ep_in,
				ptr,
				ptr_size,
				usb_resp_transfer_callback,
				rv.get(),
				usb_timeout_ms);

		success = libusb_submit_transfer(rv->resp_transfer);
		if( success != 0 ) {
			throw DeviceError(__PRETTY_FUNCTION__,
					(boost::format("submitting USB response transfer failed with error code %d")
					 % success).str());
		}

		//std::cout << "return from commit" << std::endl;
		return rv;
	}
	//---------------------------------------------------------------------------
	void
	FlyspiCom::waitRequest(FlyspiCom::RequestHandle const& handle) {
		// XXX spinlock!
		//std::cout << "waiting for request to finish" << std::endl;
		while( !handle->complete || !handle->resp_complete )
			libusb_handle_events(usb.context);
	}
	//---------------------------------------------------------------------------
	std::string
	FlyspiCom::getSerial() {
		return usb_serial;
	}
	//---------------------------------------------------------------------------

	void usb_transfer_callback(libusb_transfer* trans) {
		//std::cout << "usb_transfer_callback " << trans->status << std::endl;
		auto info = static_cast<FlyspiCom::RequestHandleInfo*>(trans->user_data);

		if( trans->status == LIBUSB_TRANSFER_COMPLETED ) {
			if( trans->actual_length != trans->length )
				throw DeviceError(__PRETTY_FUNCTION__,
						(boost::format("only %d of %d bytes where transferred")
						 % trans->actual_length % trans->length).str());
			
			info->complete = true;
		} else {
			throw DeviceError(__PRETTY_FUNCTION__,
					(boost::format("asynchronous USB transfer failed with error code %d")
					 % trans->status).str());
		}
	}

	void usb_resp_transfer_callback(libusb_transfer* trans) {
		//std::cout << "usb_resp_transfer_callback " << trans->status << std::endl;
		auto info = static_cast<FlyspiCom::RequestHandleInfo*>(trans->user_data);

		if( trans->status == LIBUSB_TRANSFER_COMPLETED ) {
			info->resp_complete = true;
		} else {
			throw DeviceError(__PRETTY_FUNCTION__,
					(boost::format("asynchronous USB transfer failed with error code %d (response)")
					 % trans->status).str());
		}
	}

} /* namespace rw_api */

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
