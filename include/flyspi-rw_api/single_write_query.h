#pragma once

#include <boost/format.hpp>

#include "flyspi-rw_api/query_base.h"
#include "flyspi-rw_api/error_base.h"

namespace rw_api {

	template<typename Com, typename Channel>
	class SingleWriteQuery : public QueryBase<Com, Channel, SingleWriteQuery<Com,Channel>> {
		public:
		typename Com::Address addr;

		SingleWriteQuery(Com& com, typename Com::Locator const& loc)
			:	QueryBase<Com, Channel, SingleWriteQuery<Com,Channel>>(com, loc, 1) {
		}

		SingleWriteQuery(SingleWriteQuery const&) = delete;
		SingleWriteQuery& operator = (SingleWriteQuery const&) = delete;

		SingleWriteQuery& iwrite(typename Com::Address const& addr, typename Com::Data const& data) {
			if( this->counter >= this->capacity ) {
				throw LogicError(__PRETTY_FUNCTION__,
						(boost::format("Too many queries (%d)! Allocated for %d queries.")
						 % (this->counter+1)
						 % this->capacity).str());
			}
			// insert write command	
			this->channel.writeQuery(this->buf, this->counter, addr, data);	
			++(this->counter);
			return *this;
		}
	};

}  /* namespace rw_api */
