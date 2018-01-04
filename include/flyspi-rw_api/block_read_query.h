#pragma once

#include "flyspi-rw_api/query_base.h"
#include "flyspi-rw_api/request.h"
#include "flyspi-rw_api/error_base.h"

namespace rw_api {

	template<typename Com, typename Channel>
	class BlockReadQuery : public QueryBase<Com, Channel, BlockReadQuery<Com,Channel>> {
		public:
		typename Com::Address baseAddr;

		BlockReadQuery(Com& com, typename Com::Locator const& loc, unsigned int const capacity)
			:	QueryBase<Com,Channel, BlockReadQuery<Com,Channel>>(com, loc, capacity) {
				this->resp_counter = capacity;
		}

		BlockReadQuery(BlockReadQuery&& tmp)
			:	QueryBase<Com,Channel,BlockReadQuery<Com,Channel>>(std::move(tmp)),
				baseAddr(tmp.baseAddr) {
		}

		BlockReadQuery(BlockReadQuery const&) = delete;
		BlockReadQuery& operator = (BlockReadQuery const&) = delete;

		BlockReadQuery& operator = (BlockReadQuery&& tmp) {
			if( this != &tmp ) {
				QueryBase<Com,Channel,BlockReadQuery<Com,Channel>>::operator = (std::move(tmp));
				baseAddr = tmp.baseAddr;
			}

			return *this;
		}

		BlockReadQuery& addr(typename Com::Address const& addr) {
			baseAddr = addr;

			for(unsigned int i=0; i<this->capacity; i++) {
				this->channel.readQuery(this->buf, i, addr);
			}

			return *this;
		}
	};

}  /* namesapce rw_api */


// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
