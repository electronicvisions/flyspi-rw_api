#pragma once

#include "flyspi-rw_api/single_write_query.h"
#include "flyspi-rw_api/single_read_query.h"

namespace rw_api {

	template<typename Com, typename Channel>
	void write(Com& com,
			typename Com::Locator const& loc,
			typename Com::Address const& addr,
			typename Com::Data const& data) {
		SingleWriteQuery<Com, Channel> q(com, loc);

		auto r = q.iwrite(addr, data).commit();
		r.wait();
	}

	template<typename Com, typename Channel>
	typename Com::Data read(Com& com,
			typename Com::Locator const& loc,
			typename Com::Address const& addr) {
		SingleReadQuery<Com, Channel> q(com, loc);

		auto r = q.iread(addr).commit();
		r.wait();
		return r[0];
	}

}  /* namespace rw_api */
