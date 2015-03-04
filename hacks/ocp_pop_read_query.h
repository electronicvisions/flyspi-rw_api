#pragma once

namespace rw_api {

	namespace flyspi {
		class OcpPopReadQuery 
			:	public QueryBase<FlyspiCom, FlyspiCom::OcpChannel, OcpPopReadQuery> {

			typedef QueryBase<FlyspiCom, FlyspiCom::OcpChannel, OcpPopReadQuery> BaseType;

			public:
			OcpPopReadQuery(FlyspiCom& com, FlyspiCom::Locator const& loc)
				:	QueryBase<FlyspiCom, FlyspiCom::OcpChannel, OcpPopReadQuery>(com, loc, 1) {

				resp_counter = 1;
			}

			OcpPopReadQuery(OcpPopReadQuery const&) = delete;
			OcpPopReadQuery& operator = (OcpPopReadQuery const&) = delete;
		};

	}  /* namespace flyspi */


	template<>
	inline void
	FlyspiCom::OcpChannel::header <flyspi::OcpPopReadQuery> 
	(BufferPtr const& ptr, flyspi::OcpPopReadQuery const* /*query*/) const {
		ptr[0] = static_cast<uint32_t>(CMD_READOCPFIFO);
		ptr[1] = 0;
	}

}  /* namespace rw_api */

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
