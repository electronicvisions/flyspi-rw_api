#pragma once

#include <boost/format.hpp>

#include "flyspi-rw_api/request.h"
#include "flyspi-rw_api/error_base.h"

namespace rw_api {

	template<typename Com, typename Channel, typename Derived>
	class QueryBase {
		public:
		Com& com;
		typename Com::Locator const& loc;
		unsigned int const capacity;
		unsigned int counter;
		unsigned int resp_counter;
		typename Com::BufferPtr buf;
		Channel channel;

		QueryBase(Com& com, typename Com::Locator const& loc, unsigned int const capacity)
			:	com(com),
				loc(loc),
				capacity(capacity),
				counter(0),
				resp_counter(0)
		{
			// XXX possibly check if loc is usable with Channel 
			if( channel.isTooLarge(capacity) ) {
				throw ImplementationError(__PRETTY_FUNCTION__,
						(boost::format("%d exceeds the maximum allowed capacity")
						 % capacity).str());
			}
			
			buf = com.allocate(channel.allocationSize(capacity));
		}

		QueryBase(QueryBase&& tmp)
			:	com(tmp.com),
				loc(tmp.loc),
				capacity(tmp.capacity),
				counter(tmp.counter),
				resp_counter(tmp.resp_counter),
				buf(std::move(tmp.buf)),
				channel(tmp.channel) {

			tmp.buf = nullptr;
			tmp.counter = 0;
			tmp.resp_counter = 0;
			const_cast<unsigned int&>(tmp.capacity) = 0;
		}


		~QueryBase() {
			if( buf != nullptr )
				com.free(buf);
		}

		QueryBase(QueryBase const&) = delete;
		QueryBase& operator=(QueryBase const&) = delete;

		QueryBase& operator = (QueryBase&& tmp) {
			if( this != &tmp ) {
				if( buf != nullptr )
					com.free(buf);

				com = tmp.com;
				const_cast<typename Com::Locator&>(loc) = const_cast<typename Com::Locator&>(tmp.loc);
				const_cast<unsigned int&>(capacity) = tmp.capacity;
				counter = tmp.counter;
				resp_counter = tmp.resp_counter;
				channel = tmp.channel;
				buf = tmp.buf;


				const_cast<unsigned int&>(tmp.capacity) = 0;
				tmp.counter = 0;
				tmp.resp_counter = 0;
				tmp.buf = nullptr;
			}
			
			return *this;
		}

		Request<Com, Channel> commit() {
			if( (counter == 0) && (resp_counter == 0) )
				throw LogicError(__PRETTY_FUNCTION__,
						"Query does not write or read data");

			channel.header(buf, static_cast<Derived*>(this));

			auto resp_buf_sz = channel.allocationSize(resp_counter);
			auto resp = com.allocate(resp_buf_sz);

			auto handle = com.commit(loc,
					buf,
					channel.allocationSize(counter),
					resp,
					resp_buf_sz);

			//return std::move(Request<Com, Channel>{ com, resp, resp_counter, handle });
			return Request<Com, Channel>{ com, resp, resp_counter, handle };
		}

		unsigned int size() const {
			return counter;
		}

		void resize(unsigned int s) {
			counter = s;
		}
	};

}  /* namespace rw_api */


namespace std {

	// STL adaptors
	template<typename Com, typename Channel, typename Derived>
	inline
	typename Com::BufferPtr begin(rw_api::QueryBase<Com, Channel, Derived>& q) {
		return q.buf + q.channel.header_size;
	}

	template<typename Com, typename Channel, typename Derived>
	inline
	typename Com::BufferPtr end(rw_api::QueryBase<Com, Channel, Derived>& q) {
		return q.buf + q.channel.header_size + q.counter;
	}

}

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
