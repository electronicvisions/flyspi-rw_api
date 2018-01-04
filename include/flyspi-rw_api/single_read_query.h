#pragma once

#include <boost/format.hpp>

#include "flyspi-rw_api/query_base.h"
#include "flyspi-rw_api/error_base.h"

namespace rw_api {

	template<typename Com, typename Channel>
	class SingleReadQuery : public QueryBase<Com, Channel, SingleReadQuery<Com,Channel>> {
		public:
		typename Com::Address addr;

		SingleReadQuery(Com& com, typename Com::Locator const& loc)
			:	QueryBase<Com, Channel, SingleReadQuery<Com,Channel>>(com, loc, 1) {
		}

		SingleReadQuery(SingleReadQuery const&) = delete;
		SingleReadQuery& operator = (SingleReadQuery const&) = delete;

		SingleReadQuery& iread(typename Com::Address const& addr) {
			if( this->counter >= this->capacity ) {
				throw LogicError(__PRETTY_FUNCTION__,
						(boost::format("Too many queries (%d)! Allocated for %d queries.")
						 % (this->counter)
						 % this->capacity).str());
			}
			// insert read command
			this->addr = addr;
			this->channel.readQuery(this->buf, this->counter, addr);
			++(this->counter);
			++(this->resp_counter);
			return *this;
		}
	};

}  /* namespace rw_api */
