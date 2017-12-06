#pragma once

#include <memory>
#include <libusb-1.0/libusb.h>
#include <rw_api/error_base.h>
#include <rw_api/byte_order.h>
#include <rw_api/usb_communication.h>

namespace rw_api
{
	class FlyspiCom {
		public:
		static int const usb_vendor_id = 0x04b4;
		static int const usb_product_id = 0x1003;
		static int const usb_timeout_ms = 10000;
		static unsigned char const ep_in = 0x86;
		static unsigned char const ep_out = 0x02;
		std::string usb_serial;

		enum UsbCommand {
			CMD_NOP,
			CMD_READMEM,		//obsolete
			CMD_WRITEMEM,		//obsolete
			CMD_READBURST,	//cmd,adr,count,data0,...
			CMD_WRITEBURST, //cmd,adr,count,data0,...
			CMD_READSTATUS, //cmd,dc,status
			CMD_WRITESTATUS,//cmd,dc,status
			CMD_READOCP,		//cmd,adr,data
			CMD_WRITEOCP,		//obsolete
			CMD_READOCPFIFO,//cmd,dc,data
			CMD_WRITEOCPBURST //cmd,dc,count,adr0,data0,...
		};

		typedef BigEndianReorder<uint32_t> Address;
		typedef BigEndianReorder<uint32_t> Data;
		//typedef uint32_t BufferType;
		typedef BigEndianReorder<uint32_t> BufferType;
		typedef BufferType* BufferPtr;


		/* The BufferType is reinterpret_cast to an unsigned char* pointer in
		 * the commit function. So it should not add any data members besides
		 * one value member of the underlying type. */
		static_assert(sizeof(uint32_t) == sizeof(BigEndianReorder<uint32_t>),
			"BigEndianReorder must have same size as underlying type");


		//struct Locator;
		// DELME
		struct Locator {
			enum /*class*/ Channel {
				_ocp,
				_sdram,
				_status
			};

			unsigned int chip_id;
			Channel channel;

			Locator& chip(unsigned int id) {
				chip_id = id;
				return *this;
			}

			Locator& ocp() {
				channel = /*Channel::*/_ocp;
				return *this;
			}

			Locator& sdram() {
				channel = /*Channel::*/_sdram;
				return *this;
			}

			Locator& status() {
				channel = /*Channel::*/_status;
				return *this;
			}
		};
		// end DELME
		struct OcpChannel;
		struct SdramChannel;

		FlyspiCom();
		FlyspiCom(std::string);

		~FlyspiCom();

		// XXX locking functionality for multi-threading
		inline Locator locate() const;
		inline BufferPtr allocate(unsigned int size);
		inline void free(BufferPtr const& handle);

#ifndef PYPLUSPLUS
		//struct RequestHandleInfo;
		// DELME
		struct RequestHandleInfo {
			libusb_transfer* transfer;
			libusb_transfer* resp_transfer;
			volatile bool complete;
			volatile bool resp_complete;

			RequestHandleInfo() {
				transfer = nullptr;
				resp_transfer = nullptr;
				complete = false;
				resp_complete = false;
			}

			~RequestHandleInfo() noexcept(false) {
				//std::cout << "deleting request handle" << std::endl;

				if( transfer != nullptr )
					libusb_free_transfer(transfer);

				if( resp_transfer != nullptr )
					libusb_free_transfer(resp_transfer);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wterminate"
				if( !complete || !resp_complete )
					/* ECM(2017-12-06) That is bad code (even if it will work
					 * if only this one throws and nothing else is happening w.r.t.
					 * stack unwinding)... I do not care for now. */
					throw LogicError(__PRETTY_FUNCTION__,
							"request destroyed before completion");
					//std::cerr << "request destroyed before completion" << std::endl;
#pragma GCC diagnostic pop
			}
		};
		// end DELME
		typedef std::unique_ptr<RequestHandleInfo> RequestHandle;
		RequestHandle commit(Locator const& loc,
				BufferPtr const& queryBuffer,
				unsigned int const queryBufferSize,
				BufferPtr const& respBuffer,
				unsigned int const respBufferSize);
		void waitRequest(RequestHandle const& handle);
		inline bool hasCompleted(RequestHandle const& handle) const;
#endif

		std::string  getSerial();

		private:
		usb_communication::context usb_context;
		usb_communication::device usb_device;
	};

}
