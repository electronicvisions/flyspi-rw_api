#pragma once

#include <iostream>

#include "flyspi-rw_api/query_base.h"
#include "flyspi-rw_api/request.h"
#include "flyspi-rw_api/error_base.h"

namespace rw_api {

	template<typename Com, typename Channel>
	class BlockWriteQuery : public QueryBase<Com, Channel, BlockWriteQuery<Com,Channel>> {
		public:
		typename Com::Address baseAddr;

		BlockWriteQuery(Com& com, typename Com::Locator const& loc, unsigned int const capacity)
			:	QueryBase<Com,Channel, BlockWriteQuery<Com,Channel>>(com, loc, capacity) {
		}

		BlockWriteQuery(BlockWriteQuery&& tmp)
			:	QueryBase<Com,Channel,BlockWriteQuery<Com,Channel>>(std::move(tmp)),
				baseAddr(tmp.baseAddr) {
		}

		BlockWriteQuery(BlockWriteQuery const&) = delete;
		BlockWriteQuery& operator = (BlockWriteQuery const&) = delete;

		BlockWriteQuery& operator = (BlockWriteQuery&& tmp) {
			if( this != &tmp ) {
				QueryBase<Com,Channel,BlockWriteQuery<Com,Channel>>::operator = (std::move(tmp));
				baseAddr = tmp.baseAddr;
			}

			return *this;
		}

		BlockWriteQuery& addr(typename Com::Address const& addr) {
			baseAddr = addr;
			return *this;
		}

		BlockWriteQuery& iwrite(typename Com::Data const& data) {
			if( this->counter >= this->capacity ) {
				throw LogicError(__PRETTY_FUNCTION__,
						(boost::format("Too many queries (%d)! Allocated for %d queries.")
						 % (this->counter+1)
						 % this->capacity).str());
			}

			this->channel.writeQuery(this->buf,
					this->counter,
					this->baseAddr+this->counter,
					data);
			++(this->counter);
			return *this;
		}
	};

}  /* namesapce rw_api */


// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
