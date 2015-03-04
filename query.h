#pragma once

#include <rw_api/query_base.h>
#include <rw_api/error_base.h>
#include <boost/format.hpp>

namespace rw_api {

	template<typename Com, typename Channel>
	class Query : public QueryBase<Com, Channel, Query<Com,Channel>> {
		public:
		Query(Com& com, typename Com::Locator const& loc, unsigned int const capacity)
			:	QueryBase<Com, Channel, Query<Com,Channel>>(com, loc, capacity) {
		}

		Query (Query const&) = delete;
		Query& operator=(Query const&) = delete;

		Query& iwrite(typename Com::Address const& addr, typename Com::Data const& data) {
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

		Query& iread(typename Com::Address const& addr) {
			if( this->counter+1 >= this->capacity ) {
				throw LogicError(__PRETTY_FUNCTION__,
						(boost::format("Too many queries (%d)! Allocated for %d queries.")
						 % (this->counter+1)
						 % this->capacity).str());
			}
			// insert read command
			this->channel.readQuery(this->buf, this->counter, addr);
			++(this->counter);
			++(this->resp_counter);
			return *this;
		}
	};

}  /* namespace rw_api */

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
