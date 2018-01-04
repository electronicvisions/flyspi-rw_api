#pragma once

#include <boost/format.hpp>

#include "flyspi-rw_api/query_base.h"
#include "flyspi-rw_api/error_base.h"

namespace rw_api {

	template<typename Com, typename Channel>
	class WriteQuery : public QueryBase<Com, Channel, WriteQuery<Com,Channel>> {
		public:
		WriteQuery(Com& com, typename Com::Locator const& loc, unsigned int const capacity)
			:	QueryBase<Com, Channel, WriteQuery<Com,Channel>>(com, loc, capacity) {
		}
		
		WriteQuery(WriteQuery const&) = delete;
		WriteQuery& operator = (WriteQuery const&) = delete;

		WriteQuery& iwrite(typename Com::Address const& addr, typename Com::Data const& data) {
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
