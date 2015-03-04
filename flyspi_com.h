#pragma once

#include <rw_api/error_base.h>
#include <rw_api/block_write_query.h>
#include <rw_api/block_read_query.h>
#include <rw_api/write_query.h>
#include <rw_api/single_read_query.h>
#include <rw_api/simple.h>
#include <rw_api/FlyspiCom.h>
#include <cstdint>
#include <sstream>
#include <iomanip>

namespace rw_api {


	struct FlyspiCom::OcpChannel {
		static unsigned int const header_size = 3;
		static unsigned int const query_size = 2;
		static unsigned int const resp_size = 2;
		static unsigned int const block_size = 512 / sizeof(BufferType);
		static unsigned int const max_size = 512 / sizeof(BufferType);

		constexpr unsigned int allocationSize(unsigned int const num_queries) {
			return ((header_size + num_queries * query_size) % block_size) ? 
				((header_size + num_queries * query_size) / block_size + 1) * block_size 
				: (header_size + num_queries * query_size);
		}

		bool isTooLarge(unsigned int const num_queries) const {
			return num_queries * query_size + header_size > max_size;
		}


		template<typename Q>
		inline void header(BufferPtr const& ptr, Q const* query) const {
			throw ImplementationError(__PRETTY_FUNCTION__,
					"missing template specialization for header()");
		}

		void writeQuery(BufferPtr const& ptr, unsigned int index, Address const& addr, Data const& data) const {
			ptr[index * query_size + header_size] = addr | (1 << 31);
			ptr[index * query_size + header_size + 1] = data;
			//ptr[2] = data; 
		}

		void readQuery(BufferPtr const& ptr, unsigned int index, Address const& addr) const {
			ptr[index * query_size + header_size] = addr & ~(1 << 31);
			ptr[index * query_size + header_size + 1] = 0;
		}

		Data& extract(BufferPtr& ptr, unsigned int /*index*/) const {
			//return ptr[index * resp_size];
			//std::cout << "extract: " << std::hex;
			//for(int i=0; i<128; i++)
				//std::cout << ptr[i] << " ";
			//std::cout << std::endl;

			return ptr[2];
		}

		Data const& extract(BufferPtr const& ptr, unsigned int /*index*/) const {
			//return ptr[index * resp_size];
			//std::cout << "extract: " << std::hex; 
			//for(int i=0; i<128; i++)
				//std::cout << ptr[i] << " ";
			//std::cout << std::endl;
			return ptr[2];
		}

		bool isGood(BufferPtr const& ptr) const {
			return !(ptr[1] & 0x80000000);
		}
	};

	struct FlyspiCom::SdramChannel {
		static unsigned int const header_size = 3;
		static unsigned int const query_size = 1;
		static unsigned int const resp_size = 1;
		static unsigned int const block_size = 512 / sizeof(BufferType);
		/** maximum size limited by FPGA implementation */
		static unsigned int const max_size = 16384 * 1024 / sizeof(BufferType);

		constexpr unsigned int allocationSize(unsigned int const num_queries) {
			return ((header_size + num_queries * query_size) % block_size) ? 
				((header_size + num_queries * query_size) / block_size + 1) * block_size 
				: (header_size + num_queries * query_size);
		}

		bool isTooLarge(unsigned int const num_queries) const {
			return num_queries + header_size > max_size;
		}

		template<typename Q>
		inline void header(BufferPtr const& ptr, Q const* query) const {
			throw ImplementationError(__PRETTY_FUNCTION__, "missing template specialization for header()");
		}

		void writeQuery(BufferPtr const& ptr, unsigned int index, Address const& /*addr*/, Data const& data) const {
			ptr[index * query_size + header_size] = data;
		}

		void readQuery(BufferPtr const& /*ptr*/, unsigned int /*index*/, Address const& /*addr*/) const {
		}

		Data& extract(BufferPtr& ptr, unsigned int index) const {
			return ptr[index * resp_size + header_size];
		}

		Data const& extract(BufferPtr const& ptr, unsigned int index) const {
			return ptr[index * resp_size + header_size];
		}

		bool isGood() const {
			return true;
		}
	};

	// INSERTME
	/*
	struct FlyspiCom::RequestHandleInfo {
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

		~RequestHandleInfo() {
			//std::cout << "deleting request handle" << std::endl;

			if( transfer != nullptr )
				libusb_free_transfer(transfer);
			
			if( resp_transfer != nullptr )
				libusb_free_transfer(resp_transfer);

			if( !complete || !resp_complete )
				throw LogicError(__PRETTY_FUNCTION__,
						"request destroyed before completion");
				//std::cerr << "request destroyed before completion" << std::endl;
		}
	};
	*/
	// end INSERTME

	//
	// Implementation of inline functions
	//

	FlyspiCom::Locator
	FlyspiCom::locate() const {
		Locator rv;
		return rv;
	}

	FlyspiCom::BufferPtr 
	FlyspiCom::allocate(unsigned int size) {
		return new BufferType [size]; 
	}

	void
	FlyspiCom::free(FlyspiCom::BufferPtr const& ptr) {
		delete [] ptr;
	}

	bool
	FlyspiCom::hasCompleted(FlyspiCom::RequestHandle const& handle) const {
		if( handle )
			return handle->complete && handle->resp_complete;
		else
			return true;
	}

	template<>
	inline void
	FlyspiCom::OcpChannel::header <WriteQuery<FlyspiCom, FlyspiCom::OcpChannel>>
	(BufferPtr const& ptr, WriteQuery<FlyspiCom, FlyspiCom::OcpChannel> const* query) const {
		ptr[0] = static_cast<uint32_t>(CMD_WRITEOCPBURST);
		ptr[1] = 0;
		ptr[2] = query->counter * query_size;
	}

	template<>
	inline void
	FlyspiCom::OcpChannel::header <SingleWriteQuery<FlyspiCom, FlyspiCom::OcpChannel>>
	(BufferPtr const& ptr, SingleWriteQuery<FlyspiCom, FlyspiCom::OcpChannel> const* query) const {
		ptr[0] = static_cast<uint32_t>(CMD_WRITEOCPBURST);
		//ptr[1] = query->addr & 0x80000000;
		ptr[1] = 0;
		ptr[2] = query->counter * query_size;
	}

	template<>
	inline void
	FlyspiCom::OcpChannel::header <SingleReadQuery<FlyspiCom, FlyspiCom::OcpChannel>>
	(BufferPtr const& ptr, SingleReadQuery<FlyspiCom, FlyspiCom::OcpChannel> const* query) const {
		ptr[0] = static_cast<uint32_t>(CMD_WRITEOCPBURST);
		ptr[1] = 0;
		ptr[2] = query->resp_counter * resp_size;
	}

	template<>
	inline void 
	FlyspiCom::SdramChannel::header <BlockWriteQuery<FlyspiCom, FlyspiCom::SdramChannel>>
	(BufferPtr const& ptr, BlockWriteQuery<FlyspiCom, FlyspiCom::SdramChannel> const* query) const {
		ptr[0] = static_cast<uint32_t>(CMD_WRITEBURST);
		ptr[1] = query->baseAddr;
		ptr[2] = query->counter;
	}

	template<>
	inline void 
	FlyspiCom::SdramChannel::header <BlockReadQuery<FlyspiCom, FlyspiCom::SdramChannel>>
	(BufferPtr const& ptr, BlockReadQuery<FlyspiCom, FlyspiCom::SdramChannel> const* query) const {
		ptr[0] = static_cast<uint32_t>(CMD_READBURST);
		ptr[1] = query->baseAddr;
		ptr[2] = query->capacity;
	}


	//
	// template abbreviations 
	//
	namespace flyspi {

		typedef FlyspiCom::Address Address;
		typedef FlyspiCom::Data Data;

		typedef BlockWriteQuery<FlyspiCom, FlyspiCom::SdramChannel> SdramBlockWriteQuery;
		typedef BlockReadQuery<FlyspiCom, FlyspiCom::SdramChannel> SdramBlockReadQuery;
		typedef WriteQuery<FlyspiCom, FlyspiCom::OcpChannel> OcpWriteQuery;
		typedef SingleReadQuery<FlyspiCom, FlyspiCom::OcpChannel> OcpSingleReadQuery;
		typedef SingleWriteQuery<FlyspiCom, FlyspiCom::OcpChannel> OcpSingleWriteQuery;

		typedef Request<FlyspiCom, FlyspiCom::OcpChannel> OcpRequest;
		typedef Request<FlyspiCom, FlyspiCom::SdramChannel> SdramRequest;

		inline void ocpWrite(FlyspiCom& com,
				FlyspiCom::Locator const& loc,
				FlyspiCom::Address const& addr,
				FlyspiCom::Data const& data) {
			write<FlyspiCom, FlyspiCom::OcpChannel>(com, loc, addr, data);
		}

		// defined in hacks/ocp_read.h
		//inline FlyspiCom::Data ocpRead(FlyspiCom& com,
				//FlyspiCom::Locator const& loc,
				//FlyspiCom::Address const& addr) {
			//return read<FlyspiCom, FlyspiCom::OcpChannel>(com, loc, addr);
		//}
	}  /* namespace flyspi */


}  /* namespace rw_api */

#include <rw_api/hacks/ocp_read.h>

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
